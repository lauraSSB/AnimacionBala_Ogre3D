#pragma once

#include <functional>
#include <string>
#include <Ogre.h>

class ThrowerAnimator
{
public:
    void build(
        Ogre::Entity* entity,
        const std::string& idleClip,
        const std::string& throwClip,
        float releaseTime
        );
    bool isBusy( ) const;
    void start( std::function< void( ) > onRelease );
    void update( float dt );
    void setSpeed( float speed );

private:
    Ogre::AnimationState*    m_Idle = nullptr;
    Ogre::AnimationState*    m_Throw = nullptr;
    float                    m_ReleaseTime = 0.3f;
    bool                     m_Playing = false;
    bool                     m_ReleasePending = false;
    std::function< void( ) > m_OnRelease;
    float                    m_Speed = 1.0f;
};