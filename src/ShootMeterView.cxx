#include "ShootMeterView.h"

#include <string>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>

namespace
{
    // Posición y tamaño de la barra (en fracción de la pantalla, 0 a 1)
    const float kBarLeft   = 0.30f;
    const float kBarTop    = 0.90f;
    const float kBarWidth  = 0.40f;
    const float kBarHeight = 0.04f;

    // Zona roja: del 35% al 65% de la barra (alrededor del centro)
    const float kRedStart = 0.35f;
    const float kRedEnd   = 0.65f;

    const float kMarkerWidth = 0.006f;

    // Crea un panel de color dentro de la barra.
    // 'start' y 'end' son fracciones de la barra (0 a 1)
    Ogre::OverlayElement* createZone(
        Ogre::OverlayContainer* bar,
        const std::string& name,
        const std::string& material,
        float start,
        float end
        )
    {
        auto& manager = Ogre::OverlayManager::getSingleton( );
        auto* zone = manager.createOverlayElement( "Panel", name );
        zone->setMetricsMode( Ogre::GMM_RELATIVE );
        zone->setPosition( start * kBarWidth, 0.0f );
        zone->setDimensions( ( end - start ) * kBarWidth, kBarHeight );
        zone->setMaterialName( material );
        bar->addChild( zone );
        return( zone );
    }
}

//--------------------------------------------------------------------------
// Arma la barra con el degradado y la marca, y los pone en un overlay
void ShootMeterView::
build( )
{
    auto& manager = Ogre::OverlayManager::getSingleton( );
    this->m_Overlay = manager.create( "ShootMeterOverlay" );

    auto* bar = static_cast< Ogre::OverlayContainer* >(
        manager.createOverlayElement( "Panel", "ShootMeter/Bar" )
        );
    bar->setMetricsMode( Ogre::GMM_RELATIVE );
    bar->setPosition( kBarLeft, kBarTop );
    bar->setDimensions( kBarWidth, kBarHeight );
    bar->setMaterialName( "ShotMeter/Gradient" );

    this->m_Marker = manager.createOverlayElement( "Panel", "ShootMeter/Marker" );
    this->m_Marker->setMetricsMode( Ogre::GMM_RELATIVE );
    this->m_Marker->setDimensions( kMarkerWidth, kBarHeight * 1.4f );
    this->m_Marker->setMaterialName( "ShotMeter/Marker" );
    bar->addChild( this->m_Marker );

    this->m_Overlay->add2D( bar );
    this->setCursor( 0.0f );
    this->m_Overlay->hide( );
}

//--------------------------------------------------------------------------
// Muestra u oculta la barra
void ShootMeterView::
setVisible( bool visible )
{
    if( visible )
        this->m_Overlay->show( );
    else
        this->m_Overlay->hide( );
}

//--------------------------------------------------------------------------
// Mueve la marca a la posición del cursor (de 0 a 1 a lo ancho de la barra)
void ShootMeterView::
setCursor( float cursor )
{
    this->m_Marker->setPosition(
        cursor * kBarWidth - kMarkerWidth * 0.5f,
        -kBarHeight * 0.2f
        );
}