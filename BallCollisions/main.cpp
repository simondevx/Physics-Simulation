#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>
#include <math.h>
#include <vector>

class Line : public sf::Drawable {
private:
    sf::Vector2f pos1 = sf::Vector2f(0.f, 0.f);
    sf::Vector2f pos2 = sf::Vector2f(0.f, 0.f);

    sf::RectangleShape body;
    sf::CircleShape startCircle;
    sf::CircleShape endCircle;
public:
    Line(sf::Vector2f start = sf::Vector2f(0.f, 0.f)) : pos1(start) {
        body.setPosition(start);
        body.setFillColor(sf::Color::White);
        body.setOutlineColor(sf::Color::White);
        body.setOrigin(sf::Vector2f(0.f, 5.f));
        
        startCircle.setRadius(5.f);
        startCircle.setOrigin(sf::Vector2f(5.f, 5.f));
        startCircle.setFillColor(sf::Color::White);
        startCircle.setPosition(pos1);
        
        endCircle.setRadius(5.f);
        endCircle.setOrigin(sf::Vector2f(5.f, 5.f));
        endCircle.setFillColor(sf::Color::White);
        endCircle.setPosition(pos2);
    }

    void setEnd(sf::Vector2f end) {
        pos2 = end;
        float length;
        sf::Angle angle;

        sf::Vector2f connection = pos2 - pos1;
        if (pos1.x == pos2.x && pos1.y == pos2.y) {
            length = 0;
            angle = sf::degrees(0);
        }
        else {
            length = connection.length();
            angle = connection.angle();
        }
        body.setSize({ length, 10.f });
        body.setRotation(angle);
        endCircle.setPosition(end);
    }

    sf::Vector2f getStart() const {
        return pos1;
    }

    sf::Vector2f getEnd() const {
        return pos2;
    }

    sf::Angle getRotation() const {
        return body.getRotation();
    }

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(body, states);
        target.draw(startCircle, states);
        target.draw(endCircle, states);
    }
};

class Ball : public sf::CircleShape {
private:
    sf::Vector2f velocity;
    float weight;
    const sf::RenderWindow& window;

    sf::Vector2f rotateVector(sf::Vector2f& oldVec, float alpha) {
        sf::Vector2f newVec(
            std::cos(alpha) * oldVec.x - std::sin(alpha) * oldVec.y, 
            std::sin(alpha) * oldVec.x + std::cos(alpha) * oldVec.y
        );
        return newVec;
    }

public:
    Ball(const sf::RenderWindow& win, 
        float radius = 25.f, 
        sf::Vector2f position = sf::Vector2f(25.0f,25.0f),
        sf::Vector2f vel = sf::Vector2f(0.0f, 0.0f),
        sf::Color color = sf::Color::White,
        std::optional<float> w = std::nullopt)
        : sf::CircleShape(radius), velocity(vel), window(win) {
        weight = w.value_or(radius*radius);
        setFillColor(color);
        setOrigin(sf::Vector2f(radius, radius));
        setPosition(position);
    }

    sf::Vector2f getVelocity() {
        return velocity;
    }
    
    void setVelocity(sf::Vector2f newVelocity) {
        
        velocity = newVelocity;
        if (newVelocity.x > 1000) {
            velocity.x = 1000;
        }
        else if (newVelocity.x < -1000) {
            velocity.x = -1000;
        }
        if (newVelocity.y > 1000) {
            velocity.y = 1000;
        }
        else if (newVelocity.y < -1000) {
            velocity.y = -1000;
        }
    }

    float getWeight() {
        return weight;
    }

    void drawVelocityArrow(sf::RenderWindow& window) {
        sf::Vector2f vel = getVelocity();
        sf::Vector2f pos = getPosition();

        std::array line =
        {
            sf::Vertex{pos},
            sf::Vertex{pos + vel * 5.0f}
        };
        line[0].color = sf::Color::Red;
        line[1].color = sf::Color::Red;
        window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
    }

    void detectWallCollision(const sf::RenderWindow& window) {
        // ball variables
        sf::Vector2f pos = getPosition();
        sf::Vector2f vel = getVelocity();
        float radius = getRadius();

        // window variables
        sf::Vector2f size(window.getSize());

        // check left side
        if (pos.x - radius <= 0) {
            pos.x = radius;
            vel.x = -vel.x; // mirror velocity
        } // check right side
        else if (pos.x + radius >= size.x) {
            pos.x = size.x - radius;
            vel.x = -vel.x; // mirror velocity
        }
        // check top side
        if (pos.y - radius <= 0) {
            pos.y = radius;
            vel.y = -vel.y; // mirror velocity
        } // check bottom side
        else if (pos.y + radius >= size.y) {
            pos.y = size.y - radius;
            vel.y = -vel.y; // mirror velocity
        }
        setPosition(pos);
        setVelocity(vel);
    }

    void detectObstacleCollision(const std::vector<Line*>* obstacles) {
        for (const Line* obstacle : *obstacles) {
            sf::Vector2f start = obstacle->getStart();
            sf::Vector2f end = obstacle->getEnd();

            sf::Vector2f ball_pos = getPosition();

            sf::Vector2f connection = end - start;

            float solution = connection.x * ball_pos.x + connection.y * ball_pos.y;
            float r = (solution - connection.x * start.x - connection.y * start.y) / (std::pow(connection.x, 2) + std::pow(connection.y, 2));


            bool collision = false;

            float collision_distance = 5.f + getRadius();

            if (r <= 0) { // first corner collision
                float distance = (ball_pos - start).length();
                if (distance <= collision_distance) {
                    sf::Vector2f collision_connection(ball_pos - start);
                    collision = true;
                    setPosition(start + collision_connection.normalized() * collision_distance);
                }
            }
            else if (r >= 1) { // second corner collision
                float distance = (ball_pos - end).length();
                if (distance <= collision_distance) {
                    sf::Vector2f collision_connection(ball_pos - end);
                    collision = true;
                    setPosition(end + collision_connection.normalized() * collision_distance);
                }
            }
            else { // body collision
                sf::Vector2f nearestPoint = start + r * connection;
                float distance = (ball_pos - nearestPoint).length();
                if (distance <= collision_distance) {
                    collision = true;
                    setPosition(nearestPoint + (ball_pos - nearestPoint).normalized() * collision_distance);
                }
            }

            if (collision) {
                float alpha = obstacle->getRotation().asRadians();

                sf::Vector2f rotatedVel = rotateVector(velocity, -alpha);

                rotatedVel = sf::Vector2f(rotatedVel.x, -rotatedVel.y);

                sf::Vector2f newVel = rotateVector(rotatedVel, alpha);

                velocity = newVel;
            }
        }
    }

    

    void addGravity(float dt) {
        const float gravity = 1.2f * dt;

        sf::Vector2f vel = getVelocity();
        setVelocity(sf::Vector2f(vel.x, vel.y + gravity));
    }

    void addResistance(float dt) {
        const float resistance = std::pow(0.999f, dt);

        sf::Vector2f vel = getVelocity();
        setVelocity(sf::Vector2f(vel.x * resistance, vel.y * resistance));
    }

    void update(const sf::Time& deltaTime) {
        float dt = deltaTime.asSeconds()*20.0f;
        addGravity(dt);
        addResistance(dt);
        move(getVelocity()*dt);
        detectWallCollision(window);
    }
};



void detectBallCollision(Ball& ball1, Ball& ball2) {
    sf::Vector2f pos1 = ball1.getPosition();
    sf::Vector2f pos2 = ball2.getPosition();

    sf::Vector2f connection = pos1 - pos2;

    if (connection.length() <= ball1.getRadius() + ball2.getRadius()) {
        float weight1 = ball1.getWeight();
        float weight2 = ball2.getWeight();

        sf::Vector2f vel1 = ball1.getVelocity();
        sf::Vector2f vel2 = ball2.getVelocity();

        

        sf::Vector2f divider = sf::Vector2f(connection.y, -connection.x);
        sf::Vector2f middle = (pos1 + pos2) / 2.0f;

        float vel1length = vel1.length();
        float vel2length = vel2.length();

        // move to balls to not be clipped
        float clipped_length = (ball1.getRadius()+ball2.getRadius())-connection.length();
        sf::Vector2f newPos1 = sf::Vector2f(pos1 + (pos1 - pos2).normalized() * clipped_length * 0.5f);
        sf::Vector2f newPos2 = sf::Vector2f(pos2 + (pos2 - pos1).normalized() * clipped_length * 0.5f);

        ball1.setPosition(newPos1);
        ball2.setPosition(newPos2);

        pos1 = newPos1;
        pos2 = newPos2;

        // update the balls velocity
        sf::Vector2f point2 = pos2 + (vel2 - vel1).normalized();
        float r2 = (connection.x * (point2.x - pos2.x) + connection.y * (point2.y - pos2.y)) / (std::pow(connection.x, 2) + std::pow(connection.y, 2));
        sf::Vector2f l2 = pos2 + r2 * connection;
        sf::Vector2f point2mirrored = l2 + (l2 - point2);
        sf::Vector2f newVel1 = (point2mirrored - pos2) * vel2length;

        sf::Vector2f point1 = pos1 + (vel1 - vel2).normalized();
        float r1 = (connection.x * (point1.x - pos1.x) + connection.y * (point1.y - pos1.y)) / (std::pow(connection.x, 2) + std::pow(connection.y, 2));
        sf::Vector2f l1 = pos1 + r1 * connection;
        sf::Vector2f point1mirrored = l1 + (l1 - point1);
        sf::Vector2f newVel2 = (point1mirrored - pos1) * vel1length;

        ball1.setVelocity(newVel1);
        ball2.setVelocity(newVel2);
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 800 }), "BallCollisions");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    std::string mode = "edit"; // edit or play
    int editMode = 0;

    sf::Clock deltaClock;

    std::vector<Ball*> balls;
    balls.push_back(new Ball(window, 50.0f, sf::Vector2f(75.0f,200.0f), sf::Vector2f(20.0f, 0.0f)));
    balls.push_back(new Ball(window, 25.0f, sf::Vector2f(155.0f,175.0f), sf::Vector2f(6.0f, -8.0f)));

    bool dragging = false;
    int selected_ball = -1;

    bool showVelocity = false;

    std::vector<Line*> obstacles;
    int selected_obstacle = -1;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (mode == "edit") {
                // editing logic
                if (!ImGui::GetIO().WantCaptureMouse) {
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                        sf::Vector2f mouse_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        if (!dragging) {
                            for (int i = 0; i < balls.size(); i++) {
                                float radius = balls[i]->getRadius();
                                sf::Vector2f pos = balls[i]->getPosition();
                                if ((mouse_pos - pos).length() < radius) {
                                    dragging = true;
                                    selected_ball = i;
                                }
                            }
                            if (editMode == 4) {
                                dragging = true;
                                obstacles.push_back(new Line(mouse_pos));
                                selected_obstacle = obstacles.size() - 1;
                            }
                        }
                        if (dragging) {
                            // checking if selected ball is valid
                            if (selected_ball >= 0 && selected_ball < balls.size()) {
                                // checking for different edit modes 0: position 1: velocity 2: size
                                if (editMode == 0) {
                                    balls[selected_ball]->setPosition(mouse_pos);
                                }
                                else if (editMode == 1) {
                                    balls[selected_ball]->setVelocity(-(mouse_pos - balls[selected_ball]->getPosition()) / 5.0f);
                                }
                                else if (editMode == 2) {
                                    sf::Vector2f ball_pos = balls[selected_ball]->getPosition();
                                    float oldRadius = balls[selected_ball]->getRadius();
                                    float newRadius = (mouse_pos - ball_pos).length();
                                    balls[selected_ball]->setOrigin(sf::Vector2f(newRadius, newRadius));
                                    balls[selected_ball]->setRadius(newRadius);
                                }
                            }
                            
                            
                            if (editMode == 4) {
                                obstacles[selected_obstacle]->setEnd(mouse_pos);
                            }
                        }
                    }
                    if (event->is<sf::Event::MouseButtonReleased>()) {
                        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        dragging = false;
                        if (editMode == 3) {
                            if (selected_ball >= 0 && selected_ball < balls.size()) {
                                if ((balls[selected_ball]->getPosition() - mousePos).length() <= balls[selected_ball]->getRadius()) {
                                    delete balls[selected_ball]; // Speicher freigeben
                                    balls.erase(balls.begin() + selected_ball);
                                    selected_ball = -1;
                                }
                                else {
                                    balls.push_back(new Ball(window, 25.0f, mousePos));
                                }
                            }
                            else {
                                balls.push_back(new Ball(window, 25.0f, mousePos));
                            }
                        }
                    }
                }
            }
        }

        sf::Time dt = deltaClock.restart();

        ImGui::SFML::Update(window, dt);

        ImGui::SetNextWindowSize(ImVec2(260, 100));
        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::Begin("Config");

        // imgui logic

        if (mode == "edit") {
            const char* editModes[] = { "Position", "Velocity", "Size", "Ball Amount", "Obstacles"};
            if (ImGui::Combo("Edit Mode", &editMode, editModes, IM_ARRAYSIZE(editModes))) {
                if (editMode == 1) {
                    showVelocity = true;
                }
                else {
                    showVelocity = false;
                }
            }
            ImGui::Checkbox("Show Velocity", &showVelocity);
            if (ImGui::Button("Simulate")) {
                mode = "play";
                editMode = 0;
            }
        }
        else if (mode == "play") {
            ImGui::Checkbox("Show Velocity", &showVelocity);
            if (ImGui::Button("Edit")) {
                mode = "edit";
            }
        }
        
        
        
        ImGui::End();

        window.clear();


        // ball logic

        for (int i = 0; i < balls.size(); i++) {
            window.draw(*balls[i]);
        }

        if (mode == "play") {
            for (int i = 0; i < balls.size(); i++) {
                balls[i]->update(dt);
                for (int j = i + 1; j < balls.size(); j++) {
                    detectBallCollision(*balls[i], *balls[j]);
                }
                balls[i]->detectObstacleCollision(&obstacles);
            }
        }

        if (showVelocity == true) {
            for (int i = 0; i < balls.size(); i++) {
                balls[i]->drawVelocityArrow(window);
            }
        }

        // obstacle drawing

        for (int i = 0; i < obstacles.size(); i++) {
            window.draw(*obstacles[i]);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}