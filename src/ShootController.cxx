#include "ShootController.h"

#include <cmath>

namespace
{
    const double kSweepSeconds = 1.0;    // segundos en recorrer la barra de un extremo al otro
    const float  kMinSpeed     = 3.0f;   // m/s con fuerza 0
    const float  kMaxSpeed     = 14.0f;  // m/s con fuerza 1
}

void ShootController::
update( double elapsedSeconds )
{
    this->m_Cursor += this->m_Direction * static_cast< float > ( elapsedSeconds / kSweepSeconds );

    if( this->m_Cursor >= 1.0f ){
        this->m_Cursor = 1.0f;
        this->m_Direction = -1.0f;
    }
    else if( this->m_Cursor <= 0.0f )
    {
        this->m_Cursor = 0.0f;
        this->m_Direction = 1.0f;
    }
}

float ShootController::
getCursor( ) const
{
    return ( this->m_Cursor );
}

float ShootController::
getPower( ) const
{
    return( 1.0f - std::fabs( 2.0f * this->m_Cursor - 1.0f ) );
}

float ShootController::
powerToSpeed( float power)
{
    return( kMinSpeed + power * ( kMaxSpeed - kMinSpeed ) );
}