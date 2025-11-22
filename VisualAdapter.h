// VisualAdapter.h
#pragma once
// Minimal, header-only adapter to map Furniture* -> SFML Rectangle shapes
// Does NOT own the Furniture* pointers (Room still owns them).

#include <SFML/Graphics.hpp>
#include "FurnitureSim.h"
#include <cmath>

struct VisualItem {
    Furniture* model;                 // pointer to existing Furniture (not owned)
    sf::RectangleShape shape;         // used for outline/selection
    sf::Sprite sprite;                // used for realistic rendering
    bool hasTexture = false;
    bool selected = false;
    bool overlapping = false;
    int rotation90 = 0; 

    // constructor: create the visual rectangle using model dims and a scale factor (pixels per model unit)
    VisualItem(Furniture* f, float scale = 1.0f, const sf::Texture* tex = nullptr) : model(f) {
        // Setup shape (bounding box/outline)
        shape.setSize(sf::Vector2f((float)f->getWidth() * scale, (float)f->getHeight() * scale));
        shape.setPosition((float)f->getX() * scale, (float)f->getY() * scale);
        shape.setOutlineThickness(2.0f);
        shape.setFillColor(sf::Color::Transparent); // Transparent fill if we have texture
        shape.setOutlineColor(sf::Color::Black);

        // Setup sprite if texture provided
        if (tex) {
            hasTexture = true;
            sprite.setTexture(*tex);
            // Calculate scale to fit the texture into the furniture dimensions
            sf::Vector2u texSize = tex->getSize();
            float sx = (f->getWidth() * scale) / (float)texSize.x;
            float sy = (f->getHeight() * scale) / (float)texSize.y;
            sprite.setScale(sx, sy);
            sprite.setPosition(shape.getPosition());
        } else {
            // Fallback color mapping
            sf::Color fillCol(200,200,255);
            switch (f->getSymbol()) {
                case 'S': fillCol = sf::Color(200,180,255); break; // Sofa
                case 'T': fillCol = sf::Color(180,255,200); break; // Table
                case 'C': fillCol = sf::Color(255,220,180); break; // Chair
                case 'B': fillCol = sf::Color(200,255,255); break; // Bed
                case 'U': fillCol = sf::Color(220,220,220); break; // Cupboard
                case 'V': fillCol = sf::Color(255,200,200); break; // TVStand
                default:  fillCol = sf::Color(200,200,255); break;
            }
            shape.setFillColor(fillCol);
        }
    }

    // Update SFML shape from model coordinates (use when model changed externally)
    void syncFromModel(float scale = 1.0f) {
        float w = (float)model->getWidth() * scale;
        float h = (float)model->getHeight() * scale;
        
        shape.setSize(sf::Vector2f(w, h));
        shape.setPosition((float)model->getX() * scale, (float)model->getY() * scale);
        
        if (hasTexture) {
            const sf::Texture* tex = sprite.getTexture();
            if (tex) {
                sf::Vector2u texSize = tex->getSize();
                sprite.setScale(w / (float)texSize.x, h / (float)texSize.y);
                sprite.setPosition(shape.getPosition());
            }
        }
    }

    // Write shape position back into model (rounding)
    void syncToModel(float scale = 1.0f) {
        sf::Vector2f p = shape.getPosition();
        int mx = (int)std::round(p.x / scale);
        int my = (int)std::round(p.y / scale);
        model->setPosition(mx, my);
    }

    bool containsPoint(const sf::Vector2f &pt) const {
        return shape.getGlobalBounds().contains(pt);
    }

    void updateOutline() {
        if (selected) {
            shape.setOutlineColor(sf::Color::Yellow);
            shape.setOutlineThickness(3.0f);
        } else if (overlapping) {
            shape.setOutlineColor(sf::Color::Red);
            shape.setOutlineThickness(3.0f);
        } else {
            shape.setOutlineColor(sf::Color::Transparent); // Hide outline if normal
            shape.setOutlineThickness(0.0f);
        }
    }
    
    void draw(sf::RenderWindow& window) {
        if (hasTexture) {
            window.draw(sprite);
        } else {
            window.draw(shape); // Draw filled shape if no texture
        }
        // Always draw outline on top if selected or overlapping
        if (selected || overlapping) {
            // Ensure shape has transparent fill so we only see outline
            sf::Color oldFill = shape.getFillColor();
            shape.setFillColor(sf::Color::Transparent);
            window.draw(shape);
            shape.setFillColor(oldFill); // Restore
        }
    }
};
