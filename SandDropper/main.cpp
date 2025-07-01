#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>
#include <vector>

#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;
using namespace sf;

struct Sandkorn
{
    float velocity = 1.f;
};

array<int, 2> checkPixel(
    int dimensions[],
    vector<vector<optional<Sandkorn>>>& screen,
    vector<vector<optional<Sandkorn>>>& screenBuffer,
    int i,
    int j) 
{
    if (j == dimensions[1] - 1) {
        return {i, j};
    }
    if (!screen[i][j + 1]) {
        return {i, j+1};
    }
    if (i == 0) {
        if (!screen[i + 1][j + 1]) {
            return {i + 1, j + 1};
        }
        return {i, j};
    }
    if (i == dimensions[0] - 1) {
        if (!screen[i - 1][j + 1]) {
            return { i - 1, j + 1 };
        }
        return {i, j};
    }
    if (!screen[i - 1][j + 1] && !screen[i + 1][j + 1]) {
        if (rand() % 2 == 0) {
            return {i-1, j+1};
        }
        else {
            return { i + 1, j + 1 };
        }
    }
    if (!screen[i - 1][j + 1]) {
        return {i-1, j+1};
    }
    if (!screen[i + 1][j + 1]) {
        return {i+1, j+1};
    }
    return { i, j };
}

int main() {

    RenderWindow window(VideoMode({ 600, 600 }), "SandDropper", Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    int dimensions[2] = { window.getSize().x, window.getSize().y };

    vector<vector<optional<Sandkorn>>> screen(dimensions[0], vector<optional<Sandkorn>>(dimensions[1], nullopt));

    sf::Clock deltaClock;


    float elapsedTime = 0; // in ms
    float timePerUpdate = 100; // in ms
    sf::Clock updateClock;

    srand(time(0));

    int brushSize = 1;
    float gravity = 0.3;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<Event::Closed>()) {
                window.close();
            }
        }

        if (Mouse::isButtonPressed(Mouse::Button::Left)) {
            if (!ImGui::GetIO().WantCaptureMouse) {
                Vector2f pos(Mouse::getPosition(window));
                if (pos.x >= 0 && pos.x < dimensions[0] && pos.y >= 0 && pos.y < dimensions[1]) {
                    screen[pos.x][pos.y] = Sandkorn{};
                    for (int i = 1; i < brushSize; i++) {
                        if (pos.x + i >= 0 && pos.x + i < dimensions[0]) {
                            screen[pos.x + i][pos.y] = Sandkorn{};
                        }
                        if (pos.x - i >= 0 && pos.x - i < dimensions[0]) {
                            screen[pos.x - i][pos.y] = Sandkorn{};
                        }
                    }
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::SetNextWindowSize(ImVec2(260, 100));
        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::Begin("Hello, world!");
        ImGui::SliderInt("Brush Size", &brushSize, 1, 5);
        ImGui::SliderFloat("Gravity", &gravity, 0.1f, 1.f);
        if (ImGui::Button("Reset")) {
            vector<vector<optional<Sandkorn>>> emptyScren(dimensions[0], vector<optional<Sandkorn>>(dimensions[1], nullopt));
            screen = emptyScren;
        }
        ImGui::End();

        window.clear();

        // update
        elapsedTime += updateClock.restart().asMilliseconds();

        if (elapsedTime >= timePerUpdate) {
            vector<vector<optional<Sandkorn>>> screenBuffer(dimensions[0], vector<optional<Sandkorn>>(dimensions[1], nullopt));

            for (int i = 0; i < dimensions[0]; i++) {
                for (int j = dimensions[1]-1; j > 0; j--) {
                    
                    if (!screen[i][j]) {
                        continue;
                    }

                    float velocity = screen[i][j]->velocity;

                    float iterations = round(velocity);

                    array<int, 2> initialPos = { i, j };
                    array<int, 2> currPos = { i, j };

                    for (int n = 0; n < iterations; n++) {
                        array<int, 2> newPos = checkPixel(dimensions, screen, screenBuffer, currPos[0], currPos[1]);
                        currPos = newPos;
                    }
                    
                    Sandkorn updatedSand;

                    if (initialPos[0] == currPos[0] && initialPos[1] == currPos[1]) {
                        updatedSand = {1.f};
                    }
                    else {
                        updatedSand = { velocity + gravity };
                    }

                    screenBuffer[currPos[0]][currPos[1]] = updatedSand;

                }
            }
            screen = screenBuffer;
        }

        // drawing
        for (int i = 0; i < dimensions[0]; i++) {
            for (int j = 0; j < dimensions[1]; j++) {
                if (screen[i][j]) {
                    RectangleShape pixel(Vector2f(1, 1));
                    pixel.setPosition(Vector2f(i, j));
                    pixel.setFillColor(Color::White);
                    window.draw(pixel);
                }
            }
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}