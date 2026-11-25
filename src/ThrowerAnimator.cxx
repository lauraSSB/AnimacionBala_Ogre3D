#include "ThrowerAnimator.h"

//--------------------------------------------------------------------------
// Prepara el lanzamiento (una sola vez) y el reposo, si existe
void ThrowerAnimator::
build(
    Ogre::Entity* entity,
    const std::string& idleClip,
    const std::string& throwClip,
    float releaseTime
    )
{
    this->m_Throw = entity->getAnimationState( throwClip );
    this->m_Throw->setLoop( false );
    this->m_Throw->setTimePosition( 0.0f );
    this->m_ReleaseTime = releaseTime;

    if( !idleClip.empty( ) )
    {
        this->m_Idle = entity->getAnimationState( idleClip );
        this->m_Idle->setLoop( true );
        this->m_Idle->setEnabled( true );
        this->m_Throw->setEnabled( false );
    }
    else
        this->m_Throw->setEnabled( true ); 
}

//--------------------------------------------------------------------------
bool ThrowerAnimator::
isBusy( ) const
{
    return( this->m_Playing );
}

//--------------------------------------------------------------------------
// Empieza el lanzamiento; onRelease se llama cuando el objeto debe salir
void ThrowerAnimator::
start( std::function< void( ) > onRelease )
{
    if( this->m_Playing )
        return;

    this->m_OnRelease      = onRelease;
    this->m_ReleasePending = true;
    this->m_Playing        = true;

    if( this->m_Idle != nullptr )
        this->m_Idle->setEnabled( false );
    this->m_Throw->setTimePosition( 0.0f );
    this->m_Throw->setEnabled( true );
}

//--------------------------------------------------------------------------
void ThrowerAnimator::
update( float dt )
{
    dt *= this->m_Speed;
    if( this->m_Throw == nullptr )
        return;

    if( !this->m_Playing )
    {
        if( this->m_Idle != nullptr )
            this->m_Idle->addTime( dt );
        return;
    }

    this->m_Throw->addTime( dt );

    const bool ended = this->m_Throw->hasEnded( );
    if( this->m_ReleasePending
        && ( ended || this->m_Throw->getTimePosition( ) >= this->m_ReleaseTime ) )
    {
        this->m_ReleasePending = false;
        if( this->m_OnRelease )
            this->m_OnRelease( );
    }

    if( ended )
    {
        this->m_Playing = false;
        if( this->m_Idle != nullptr )
        {
            this->m_Throw->setEnabled( false );
            this->m_Idle->setEnabled( true );
        }
        else
            this->m_Throw->setTimePosition( 0.0f ); 
    }
}

//--------------------------------------------------------------------------
void ThrowerAnimator::
setSpeed( float speed )
{
    this->m_Speed = speed;
}