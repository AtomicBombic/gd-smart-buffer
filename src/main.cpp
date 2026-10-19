#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

// Globale variabler til at holde styr på vores smarte buffer
bool g_isSmartBuffering = false;
int g_ticksSinceClick = 0;

// Funktion til at hente den aktuelle tick-grænse fra dine indstillinger
int getSmartBufferWindow() {
    // Tjek om moddet overhovedet er slået til i menuen
    if (!Mod::get()->getSettingValue<bool>("enabled")) {
        return 0; // 0 betyder slået fra
    }

    // Hent strengen ("Off", "2", "4" osv.) fra din "window" dropdown
    std::string windowStr = Mod::get()->getSettingValue<std::string>("window");
    
    if (windowStr == "Off") return 0;
    
    try {
        return std::stoi(windowStr); // Konverterer f.eks. "4" til tallet 4
    } catch (...) {
        return 4; // Fallback hvis noget går galt
    }
}

class $modify(MyPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        // Kør spillets normale kode først (så normale klik stadig går igennem med det samme)
        PlayerObject::pushButton(button);
        
        if (button == PlayerButton::Jump && getSmartBufferWindow() > 0) {
            g_isSmartBuffering = true;
            g_ticksSinceClick = 0;
        }
    }

    void releaseButton(PlayerButton button) {
        PlayerObject::releaseButton(button);
        if (button == PlayerButton::Jump) {
            g_isSmartBuffering = false;
        }
    }
};

class $modify(MyBaseGameLayer, GJBaseGameLayer) {
    void updatePhysics(float dt) {
        // Lad spillet køre sit normale fysik-tick
        GJBaseGameLayer::updatePhysics(dt);

        int maxTicks = getSmartBufferWindow();

        // Hvis smart buffer er aktiv og ikke slået fra i menuen
        if (g_isSmartBuffering && maxTicks > 0) {
            g_ticksSinceClick++;

            // Hvis vi overskrider den valgte grænse (f.eks. 4 ticks), sletter vi klikket
            if (g_ticksSinceClick > maxTicks) {
                g_isSmartBuffering = false;
                return;
            }

            auto player = m_player1;
            if (player) {
                // Tjek om spilleren LIGE NU rører jorden eller er ved en orb
                if (player->m_canJump || player->m_isPassingOrb) { 
                    // Tving spillet til at udføre klikket præcis her på hitboxen!
                    player->pushButton(PlayerButton::Jump);
                    g_isSmartBuffering = false; // Nulstil, da klikket er brugt
                }
            }
        }
    }
};
