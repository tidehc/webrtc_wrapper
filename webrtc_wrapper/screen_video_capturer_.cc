/* Copyright(C) 2014-2015 Doubango Telecom <https://github.com/sarandogou/webrtc-everywhere> */
#include "screen_video_capturer_.hpp"

#include "webrtc/base/bind.h"
#include "talk/media/base/videocapturer.h"
#include "talk/media/webrtc/webrtcvideoframefactory.h"
#include "webrtc/modules/desktop_capture/shared_memory.h"
#include "webrtc/modules/desktop_capture/screen_capturer.h"
#include "webrtc/modules/desktop_capture/desktop_region.h"
#include "webrtc/modules/desktop_capture/desktop_frame.h"

#if WE_UNDER_APPLE
#import <ApplicationServices/ApplicationServices.h>
#define MAX_DISPLAYS    32
#endif /* WE_UNDER_APPLE */

#if !defined(kDoubangoSharedMemoryId)
#	define kDoubangoSharedMemoryId 85697421
#endif /* kDoubangoSharedMemoryId */


class _SharedMemory : public webrtc::SharedMemory {
public:
	_SharedMemory(char* buffer, size_t size)
		: SharedMemory(buffer, size, 0, kDoubangoSharedMemoryId),
		buffer_(buffer) {
	}
	virtual ~_SharedMemory() {
		if (buffer_) delete[] buffer_;
	}
private:
	char* buffer_;
	DISALLOW_COPY_AND_ASSIGN(_SharedMemory);
};

class _ScreenVideoCapturer;

class _ScreenVideoCapturerThread
	: public rtc::Thread, public rtc::MessageHandler{
public:
	explicit _ScreenVideoCapturerThread(_ScreenVideoCapturer* capturer);

	virtual ~_ScreenVideoCapturerThread();

	// Override virtual method of parent Thread. Context: Worker Thread.
	virtual void Run();

	// Override virtual method of parent MessageHandler. Context: Worker Thread.
	virtual void OnMessage(rtc::Message* /*pmsg*/);

	// Check if Run() is finished.
	bool Finished() const;

private:
	_ScreenVideoCapturer* capturer_;
	mutable rtc::CriticalSection crit_;
	bool finished_;

	DISALLOW_COPY_AND_ASSIGN(_ScreenVideoCapturerThread);
};

//
//	_ScreenVideoCapturer
//

// Fake video capturer that allows the test to manually pump in frames.
// https://chromium.googlesource.com/external/webrtc/trunk/talk/+/master/app/webrtc/objc/avfoundationvideocapturer.h
// https://chromium.googlesource.com/external/webrtc/+/master/talk/app/webrtc/objc/avfoundationvideocapturer.mm#
class _ScreenVideoCapturer : public cricket::VideoCapturer, public webrtc::ScreenCapturer::Callback {
public:
	_ScreenVideoCapturer()
		: running_(false),
		capture_(webrtc::ScreenCapturer::Create()),
		capture_thread_(NULL),
		startThread_(NULL),
		rotation_(webrtc::kVideoRotation_0) {

#ifdef HAVE_WEBRTC_VIDEO
		set_frame_factory(new cricket::WebRtcVideoFrameFactory());
#endif

		memset(&curr_frame_, 0, sizeof(curr_frame_));

		// Default supported formats. Use ResetSupportedFormats to over write.
		std::vector<cricket::VideoFormat> formats;
		static const int kDefaultScreencastFps = 5;
#if WE_UNDER_WINDOWS
		static enum cricket::FourCC defaultFourCC = cricket::FOURCC_ARGB;
		formats.push_back(cricket::VideoFormat(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			cricket::VideoFormat::FpsToInterval(kDefaultScreencastFps), defaultFourCC));
#elif WE_UNDER_APPLE
		static enum cricket::FourCC defaultFourCC = cricket::FOURCC_ARGB;
        CGDisplayCount displayCount;
        CGDirectDisplayID displays[MAX_DISPLAYS];
        if (CGGetActiveDisplayList(sizeof(displays)/sizeof(displays[0]), displays, &displayCount) == kCGErrorSuccess) {
            for (CGDisplayCount i=0; i<displayCount; i++) {
                // make a snapshot of the current display
                // CGImageRef image = CGDisplayCreateImage(displays[i]);
                formats.push_back(cricket::VideoFormat((int)CGDisplayPixelsWide(displays[i]), (int)CGDisplayPixelsHigh(displays[i]),
                                                       cricket::VideoFormat::FpsToInterval(kDefaultScreencastFps), defaultFourCC));
            }
    }
#endif
		formats.push_back(cricket::VideoFormat(1280, 720,
			cricket::VideoFormat::FpsToInterval(kDefaultScreencastFps), defaultFourCC));
		formats.push_back(cricket::VideoFormat(640, 480,
			cricket::VideoFormat::FpsToInterval(kDefaultScreencastFps), defaultFourCC));
		formats.push_back(cricket::VideoFormat(320, 240,
			cricket::VideoFormat::FpsToInterval(kDefaultScreencastFps), defaultFourCC));
		formats.push_back(cricket::VideoFormat(160, 120,
			cricket::VideoFormat::FpsToInterval(kDefaultScreencastFps), defaultFourCC));
		ResetSupportedFormats(formats);
	}

	~_ScreenVideoCapturer() {
		SignalDestroyed(this);
		capture_ = NULL;
		capture_thread_ = NULL;
		next_frame_ = NULL;
		/* curr_frame_ is a struct without allocated pointers */
	}

	void ResetSupportedFormats(const std::vector<cricket::VideoFormat>& formats) {
		SetSupportedFormats(formats);
	}
	bool GetNextFrame(int* waiting_time_ms)
	{
		CaptureFrame();
		*waiting_time_ms = 1000 / GetCaptureFormat()->framerate();
		return true;
	}
	bool CaptureFrame() {
		capture_->Capture(webrtc::DesktopRegion());
		return IsRunning();
	}

	sigslot::signal1<_ScreenVideoCapturer*> SignalDestroyed;

	virtual cricket::CaptureState Start(const cricket::VideoFormat& format) {
		if (running_) {
			LOG(LS_ERROR) << "Screen video capturer '" << GetId() << "' already running";
			SetCaptureState(cricket::CS_FAILED);
			return cricket::CS_FAILED;
		}

		cricket::VideoFormat supported;
		if (GetBestCaptureFormat(format, &supported)) {
			SetCaptureFormat(&supported);
		}

		// Create the capture thread
		capture_thread_ = new _ScreenVideoCapturerThread(this);

		SetCaptureState(cricket::CS_RUNNING);
		capture_->Start(this);

		// Keep track of which thread capture started on. This is the thread that
		// frames need to be sent to.
		DCHECK(!startThread_);
		startThread_ = rtc::Thread::Current();

		bool ret = capture_thread_->Start();
		if (ret) {
			LOG(LS_INFO) << "Screen video capturer '" << GetId() << "' started";
		}
		else {
			LOG(LS_ERROR) << "Screen video capturer '" << GetId() << "' failed to start";
			SetCaptureState(cricket::CS_FAILED);
			SetCaptureFormat(NULL);
			return cricket::CS_FAILED;
		}

		running_ = true;
		return cricket::CS_RUNNING;
	}
	virtual void Stop() {
		running_ = false;
		if (capture_thread_) {
			capture_thread_->Stop();
			capture_thread_ = NULL;
			LOG(LS_INFO) << "Screen video capturer '" << GetId() << "' stopped";
		}
		SetCaptureFormat(NULL);
		SetCaptureState(cricket::CS_STOPPED);
		startThread_ = NULL;
	}
	virtual bool IsRunning() {
		return running_ && capture_thread_ && !capture_thread_->Finished();
	}
	virtual bool IsScreencast() const { return true; }
	bool GetPreferredFourccs(std::vector<uint32>* fourccs) {
		fourccs->push_back(cricket::FOURCC_I420);
		fourccs->push_back(cricket::FOURCC_ARGB);
		fourccs->push_back(cricket::FOURCC_RGBA);
		return true;
	}

	void SetRotation(webrtc::VideoRotation rotation) {
		rotation_ = rotation;
	}

	void SetNextFrame(webrtc::DesktopFrame* next_frame) {
		next_frame_.reset(next_frame);
	}

	webrtc::VideoRotation GetRotation() { return rotation_; }

	// webrtc::ScreenCapturer::Callback::CreateSharedMemory
	virtual webrtc::SharedMemory* CreateSharedMemory(size_t size)
	{
		return new _SharedMemory(new char[size], size);
	}

	// webrtc::ScreenCapturer::Callback::OnCaptureCompleted
	virtual void OnCaptureCompleted(webrtc::DesktopFrame* desktopFrame)
	{
		if (IsRunning() && desktopFrame) {
			uint32 data_size = 0;
			if (!GetCaptureFormat()) {
				LOG(LS_ERROR) << "Screen video capturer '" << GetId() << "' GetCaptureFormat failed";
				goto next;
			}
			if (GetCaptureFormat()->fourcc == cricket::FOURCC_ARGB || GetCaptureFormat()->fourcc == cricket::FOURCC_ARGB) {
				data_size = desktopFrame->size().width() * 4 * desktopFrame->size().height();
			}
			else if (GetCaptureFormat()->fourcc == cricket::FOURCC_I420) {
				data_size = static_cast<uint32>(cricket::VideoFrame::SizeOf(desktopFrame->size().width(), desktopFrame->size().height()));
			}
			else {
				LOG(LS_ERROR) << "Screen video capturer '" << GetId() << "' Unsupported FOURCC " << GetCaptureFormat()->fourcc;
				goto next;
			}
			curr_frame_.width = desktopFrame->size().width();
			curr_frame_.height = desktopFrame->size().height();
			curr_frame_.fourcc = GetCaptureFormat()->fourcc;
			curr_frame_.data_size = data_size;
			curr_frame_.data = desktopFrame->data();

			if (startThread_->IsCurrent()) {
				SignalFrameCaptured(this, &curr_frame_);
			}
			else {
				startThread_->Invoke<void>(rtc::Bind(&_ScreenVideoCapturer::SignalFrameCapturedOnStartThread, this, &curr_frame_));
			}
		}
	next:
		SetNextFrame(desktopFrame);
	}

	// Used to signal frame capture on the thread that capturer was started on.
	void SignalFrameCapturedOnStartThread(const cricket::CapturedFrame* frame) {
		DCHECK(startThread_->IsCurrent());
		SignalFrameCaptured(this, frame);
	}

private:
	bool running_;
	webrtc::VideoRotation rotation_;
	rtc::scoped_ptr<webrtc::ScreenCapturer> capture_;
	_ScreenVideoCapturerThread* capture_thread_;
	rtc::scoped_ptr<webrtc::DesktopFrame> next_frame_;
	cricket::CapturedFrame curr_frame_;
	rtc::Thread* startThread_;  // Set in Start(), unset in Stop().
	DISALLOW_COPY_AND_ASSIGN(_ScreenVideoCapturer);
};

//
//	_ScreenVideoCapturerFactory
//
_ScreenVideoCapturerFactory::_ScreenVideoCapturerFactory()
	: window_capturer_(NULL),
	capture_state_(cricket::CS_STOPPED) {
}

cricket::VideoCapturer* _ScreenVideoCapturerFactory::Create(const cricket::ScreencastId& window) {
	if (window_capturer_ != NULL) {
		return NULL;
	}
	window_capturer_ = new _ScreenVideoCapturer;
	window_capturer_->SignalDestroyed.connect(
		this,
		&_ScreenVideoCapturerFactory::OnWindowCapturerDestroyed);
	window_capturer_->SignalStateChange.connect(
		this,
		&_ScreenVideoCapturerFactory::OnStateChange);
	return window_capturer_;
}

_ScreenVideoCapturer* _ScreenVideoCapturerFactory::window_capturer() {
	return window_capturer_;
}

cricket::CaptureState _ScreenVideoCapturerFactory::capture_state() {
	return capture_state_;
}

void _ScreenVideoCapturerFactory::OnWindowCapturerDestroyed(_ScreenVideoCapturer* capturer) {
	if (capturer == window_capturer_) {
		window_capturer_ = NULL;
	}
}

void _ScreenVideoCapturerFactory::OnStateChange(cricket::VideoCapturer*, cricket::CaptureState state) {
	capture_state_ = state;
}


//
// _ScreenVideoCapturerThread
//
_ScreenVideoCapturerThread::_ScreenVideoCapturerThread(_ScreenVideoCapturer* capturer)
	: capturer_(capturer),
	finished_(false) {
}

_ScreenVideoCapturerThread:: ~_ScreenVideoCapturerThread() {
	Stop();
}

// Override virtual method of parent Thread. Context: Worker Thread.
void _ScreenVideoCapturerThread::Run() {
	// Read the first frame and start the message pump. The pump runs until
	// Stop() is called externally or Quit() is called by OnMessage().
	int waiting_time_ms = 0;
	if (capturer_ && capturer_->GetNextFrame(&waiting_time_ms)) {
		PostDelayed(waiting_time_ms, this);
		Thread::Run();
	}

	rtc::CritScope cs(&crit_);
	finished_ = true;
}

// Override virtual method of parent MessageHandler. Context: Worker Thread.
void _ScreenVideoCapturerThread::OnMessage(rtc::Message* /*pmsg*/) {
	int waiting_time_ms = 0;
	if (capturer_ && capturer_->GetNextFrame(&waiting_time_ms)) {
		PostDelayed(waiting_time_ms, this);
	}
	else {
		Quit();
	}
}

// Check if Run() is finished.
bool _ScreenVideoCapturerThread::Finished() const {
	rtc::CritScope cs(&crit_);
	return finished_;
}