#pragma once
#include <SFML/Graphics.hpp>

class Button : public sf::Drawable {
public:
    Button(const sf::Font& font, const std::string& text, sf::Vector2f pos, sf::Vector2f size)
    {
        // background
        background.setSize(size);
        background.setPosition(pos);
        background.setFillColor(sf::Color(70, 70, 70));

        // border
        background.setOutlineColor(sf::Color::White);
        background.setOutlineThickness(2);

        // text
        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(20);
        label.setFillColor(sf::Color::White);

        // center text inside the button
        sf::FloatRect tb = label.getLocalBounds();
        label.setPosition(
            pos.x + (size.x - tb.width) / 2.f,
            pos.y + (size.y - tb.height) / 2.f - tb.top
        );
    }

    // Detecteaza daca mouse-ul e peste buton
    bool isHovered(sf::Vector2i mousePos) const {
        return background.getGlobalBounds().contains(
            (float)mousePos.x,
            (float)mousePos.y
        );
    }

    // Detecteaza click stanga
    bool isClicked(const sf::Event& event, sf::Vector2i mousePos) const {
        if (!event.is<sf::Event::MouseButtonPressed>()) return false;

        auto& e = event.get<sf::Event::MouseButtonPressed>();

        return e.button == sf::Mouse::Left &&
            isHovered(mousePos);
    }

private:
    sf::RectangleShape background;
    sf::Text label;

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const override {
        target.draw(background, states);
        target.draw(label, states);
    }
};
