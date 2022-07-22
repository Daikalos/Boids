#pragma once

#include <SFML/Audio.hpp>

class CustomRecorder : public sf::SoundRecorder
{
public:
	~CustomRecorder()
	{
		stop();
	}

    virtual bool onStart()
    {
        
        return true;
    }
    virtual bool onProcessSamples(const sf::Int16* samples, std::size_t sampleCount)
    {
        return true;
    }
    virtual void onStop()
    {

    }
};
