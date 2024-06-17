#ifndef MSC_TEST_MEDIA_STREAM_TRACK_FACTORY_HPP
#define MSC_TEST_MEDIA_STREAM_TRACK_FACTORY_HPP

#include "api/media_stream_interface.h"
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
#include <iostream>


class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance; // 唯一实例
        return instance;
    }

    // 防止拷贝和赋值
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

	void createFactory();

	void ReleaseThreads();


	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> Factory;
	mediasoupclient::PeerConnection::Options PeerConnectionOptions;

private:
    Singleton() {
        std::cout << "Singleton created\n";
        createFactory();
    }
    ~Singleton() {
        std::cout << "Singleton destroyed\n";
        ReleaseThreads();
    }

	/* MediaStreamTrack holds reference to the threads of the PeerConnectionFactory.
	* Use plain pointers in order to avoid threads being destructed before tracks.
	*/
	std::unique_ptr<rtc::Thread> NetworkThread;
	std::unique_ptr<rtc::Thread> WorkerThread;
	std::unique_ptr<rtc::Thread> SignalingThread;
};


rtc::scoped_refptr<webrtc::AudioTrackInterface> createAudioTrack(const std::string& label);

rtc::scoped_refptr<webrtc::VideoTrackInterface> createVideoTrack(const std::string& label);

#endif
