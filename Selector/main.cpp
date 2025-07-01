#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <windows.h>

int main() {
    sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "Physics Simulation");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    bool keepOpen = true;

    sf::Clock deltaClock;
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(640, 480));
        ImGui::Begin(
            "MainOverlay",
            nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings
        );
        
        ImGui::SetWindowFontScale(2.0f);
        ImGui::Text("Choose a Simulation");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Checkbox("Keep Selector open", &keepOpen);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        if (ImGui::Button("Ball Collisions")) {
            ImGui::SFML::Shutdown();
            window.close();
            if (keepOpen) {
                system("start Selector.exe");
            }
            system("start BallCollisions.exe");
        }
        if (ImGui::Button("Sand Dropper")) {
            ImGui::SFML::Shutdown();
            window.close();
            if (keepOpen) {
                system("start Selector.exe");
            }
            system("start SandDropper.exe");
        }

        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}