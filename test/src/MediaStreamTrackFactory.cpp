#define MSC_CLASS "MediaStreamTrackFactory"

#include "MediaStreamTrackFactory.hpp"
#include "MediaSoupClientErrors.hpp"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "pc/test/fake_audio_capture_module.h"
#include "pc/test/fake_video_track_source.h"
#include "mediasoupclient.hpp"

using namespace mediasoupclient;

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> Factory;

/* MediaStreamTrack holds reference to the threads of the PeerConnectionFactory.
 * Use plain pointers in order to avoid threads being destructed before tracks.
 */
std::unique_ptr<rtc::Thread> NetworkThread;
std::unique_ptr<rtc::Thread> WorkerThread;
std::unique_ptr<rtc::Thread> SignalingThread;
mediasoupclient::PeerConnection::Options PeerConnectionOptions;

void createFactory()
{
	if(Factory)
		return;
	NetworkThread   = rtc::Thread::CreateWithSocketServer();
	WorkerThread    = rtc::Thread::Create();
	SignalingThread = rtc::Thread::Create();

	NetworkThread->SetName("network_thread", nullptr);
	SignalingThread->SetName("signaling_thread", nullptr);
	WorkerThread->SetName("worker_thread", nullptr);

	if (!NetworkThread->Start() || !SignalingThread->Start() || !WorkerThread->Start())
	{
		MSC_THROW_INVALID_STATE_ERROR("thread start errored");
	}

	webrtc::PeerConnectionInterface::RTCConfiguration config;

	auto fakeAudioCaptureModule = FakeAudioCaptureModule::Create();
	if (!fakeAudioCaptureModule)
	{
		MSC_THROW_INVALID_STATE_ERROR("audio capture module creation errored");
	}

	Factory = webrtc::CreatePeerConnectionFactory(
	  NetworkThread.get(),
	  WorkerThread.get(),
	  SignalingThread.get(),
	  fakeAudioCaptureModule,
	  webrtc::CreateBuiltinAudioEncoderFactory(),
	  webrtc::CreateBuiltinAudioDecoderFactory(),
	  webrtc::CreateBuiltinVideoEncoderFactory(),
	  webrtc::CreateBuiltinVideoDecoderFactory(),
	  nullptr /*audio_mixer*/,
	  nullptr /*audio_processing*/);

	if (!Factory)
	{
		MSC_THROW_ERROR("error ocurred creating peerconnection factory");
	}
	PeerConnectionOptions.factory = Factory.get();
}

void ReleaseThreads() {
    // 调用 reset 函数以释放资源
	if (Factory) {
		// Factory->Release();
		PeerConnectionOptions.factory = nullptr;
		Factory = nullptr;
	}
    // if (NetworkThread) {
    //     NetworkThread->Stop();
    //     NetworkThread.reset();
	// 	NetworkThread = nullptr;
    // }

    // if (WorkerThread) {
    //     WorkerThread->Stop();
    //     WorkerThread.reset();
	// 	WorkerThread = nullptr;
    // }

    // if (SignalingThread) {
    //     SignalingThread->Stop();
    //     SignalingThread.reset();
	// 	SignalingThread = nullptr;
    // }

}

// Audio track creation.
rtc::scoped_refptr<webrtc::AudioTrackInterface> createAudioTrack(const std::string& label)
{
	if (!Factory)
		createFactory();

	cricket::AudioOptions options;
	options.highpass_filter = false;

	rtc::scoped_refptr<webrtc::AudioSourceInterface> source = Factory->CreateAudioSource(options);

	return Factory->CreateAudioTrack(label, source.get());
}

// Video track creation.
rtc::scoped_refptr<webrtc::VideoTrackInterface> createVideoTrack(const std::string& label)
{
	if (!Factory)
		createFactory();

	rtc::scoped_refptr<webrtc::FakeVideoTrackSource> source = webrtc::FakeVideoTrackSource::Create();

	return Factory->CreateVideoTrack(source, label);
}
