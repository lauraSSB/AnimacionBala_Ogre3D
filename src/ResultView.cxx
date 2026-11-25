#include "ResultView.h"

#include <cstdio>
#include <string>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>

#include <memory>
#include <OgreFont.h>
#include <OgreFontManager.h>
#include <OgreResourceGroupManager.h>

namespace
{
    // Posición y tamaño del panel (en fracción de la pantalla, 0 a 1)
    const float kPanelLeft   = 0.30f;
    const float kPanelTop    = 0.05f;
    const float kPanelWidth  = 0.40f;
    const float kPanelHeight = 0.12f;

    const float kCharHeight  = 0.07f;
}

//--------------------------------------------------------------------------
// Arma el panel de fondo y el texto, y los pone en un overlay
void ResultView::
build( )
{
    auto& fonts = Ogre::FontManager::getSingleton( );
    if( !fonts.resourceExists( "Resultado/Fuente" ) )
    {
        auto font = std::static_pointer_cast< Ogre::Font >(
            fonts.create( "Resultado/Fuente",
                          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME )
            );
        font->setType( Ogre::FT_TRUETYPE );
        font->setSource( "DejaVuSans-Bold.ttf" );
        font->setTrueTypeSize( 48 );
        font->setTrueTypeResolution( 96 );
        font->load( );
    }

    auto& manager = Ogre::OverlayManager::getSingleton( );
    this->m_Overlay = manager.create( "ResultOverlay" );

    auto* panel = static_cast< Ogre::OverlayContainer* >(
        manager.createOverlayElement( "Panel", "Result/Panel" )
        );
    panel->setMetricsMode( Ogre::GMM_RELATIVE );
    panel->setPosition( kPanelLeft, kPanelTop );
    panel->setDimensions( kPanelWidth, kPanelHeight );
    panel->setMaterialName( "ShotMeter/Back" );

    this->m_Text = manager.createOverlayElement( "TextArea", "Result/Text" );
    this->m_Text->setMetricsMode( Ogre::GMM_RELATIVE );
    this->m_Text->setPosition( kPanelWidth * 0.5f, 0.02f );
    this->m_Text->setParameter( "font_name", "Resultado/Fuente" );
    this->m_Text->setParameter( "char_height", std::to_string( kCharHeight ) );
    this->m_Text->setParameter( "alignment", "center" );
    this->m_Text->setColour( Ogre::ColourValue( 1.0f, 1.0f, 1.0f ) );
    panel->addChild( this->m_Text );

    this->m_Overlay->add2D( panel );
    this->m_Overlay->hide( );
}

//--------------------------------------------------------------------------
void ResultView::
show( float distance )
{
    char text[ 64 ];
    std::snprintf( text, sizeof( text ), "Distancia: %.2f m", distance );
    this->m_Text->setCaption( text );
    this->m_Overlay->show( );
}

//--------------------------------------------------------------------------
void ResultView::
hide( )
{
    this->m_Overlay->hide( );
}