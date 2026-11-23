#ifndef __ShootMeterView__h__
#define __ShootMeterView__h__

namespace Ogre { class Overlay; class OverlayElement; }

// Dibuja en pantalla la barra de fuerza: zonas verde-rojo-verde y una marca
// que se mueve con el cursor. Solo dibuja, no decide nada del tiro.
class ShootMeterView
{
public:
    using Self = ShootMeterView;

public:
    ShootMeterView( ) = default;
    ~ShootMeterView( ) = default;

    // Crea los paneles de la barra. Llamar una vez, con el overlay system ya activo
    void build( );

    // Muestra u oculta la barra
    void setVisible( bool visible );

    // Mueve la marca a la posición del cursor, de 0 a 1
    void setCursor( float cursor );

private:
    Ogre::Overlay*        m_Overlay = nullptr;
    Ogre::OverlayElement* m_Marker  = nullptr;
};

#endif