#ifndef __ShootMeterView__h__
#define __ShootMeterView__h__

namespace Ogre { class Overlay; class OverlayElement; }

class ShootMeterView
{
public:
    using Self = ShootMeterView;

public:
    ShootMeterView( ) = default;
    ~ShootMeterView( ) = default;

    void build( );

    void setVisible( bool visible );

    void setCursor( float cursor );

private:
    Ogre::Overlay*        m_Overlay = nullptr;
    Ogre::OverlayElement* m_Marker  = nullptr;
};

#endif