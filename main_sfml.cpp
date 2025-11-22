// main_sfml.cpp
// SFML-based visual front-end for your FurnitureSim logic with JSON save/load and rotation.
// Build with:
// g++ -std=c++17 main_sfml.cpp FurnitureSim.cpp -I. -o furnsim_sfml -lsfml-graphics -lsfml-window -lsfml-system
// Requires nlohmann/json.hpp available in include path.

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include "FurnitureSim.h"
#include "VisualAdapter.h"

// JSON library - nlohmann/json single-header required
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// --- small factory to create furniture by exact name (we persist names)
Furniture* createFurnitureFromName(const std::string &name, int w, int h, int x, int y) {
    if (name == "Sofa") return new Sofa(w,h,x,y);
    if (name == "Table") return new Table(w,h,x,y);
    if (name == "Chair") return new Chair(w,h,x,y);
    if (name == "Bed") return new Bed(w,h,x,y);
    if (name == "Cupboard") return new Cupboard(w,h,x,y);
    if (name == "TVStand") return new TVStand(w,h,x,y);
    // fallback: simple furniture
    return new SimpleFurniture(name, w, h, x, y, 'X');
}

// Save layout to JSON file
bool saveLayoutJSON(const std::string &filename, const Room &room) {
    json root;
    root["room_width"] = room.getWidth();
    root["room_height"] = room.getHeight();
    root["items"] = json::array();

    for (int i = 0; i < room.count(); ++i) {
        Furniture* f = room.getFurnitureAt(i);
        if (!f) continue;
        json item;
        item["name"] = f->getName();
        item["width"] = f->getWidth();
        item["height"] = f->getHeight();
        item["x"] = f->getX();
        item["y"] = f->getY();
        // symbol included for completeness
        item["symbol"] = std::string(1, f->getSymbol());
        root["items"].push_back(item);
    }

    std::ofstream ofs(filename);
    if (!ofs) return false;
    ofs << root.dump(4) << std::endl;
    return true;
}

// Load layout from JSON file (clears room and rebuilds)
bool loadLayoutJSON(const std::string &filename, Room &room) {
    std::ifstream ifs(filename);
    if (!ifs) return false;

    json root;
    try {
        ifs >> root;
    } catch (...) {
        return false;
    }

    // Validate
    if (!root.contains("items")) return false;

    // Before loading, remove all existing items from room safely
    // Remove last->first to keep indices valid in Room::removeFurniture
    int currentCount = room.count();
    for (int i = currentCount - 1; i >= 0; --i) {
        room.removeFurniture(i); // deletes pointers in Room
    }

    // Create items from JSON
    for (const auto &it : root["items"]) {
        std::string name = it.value("name", std::string("Unknown"));
        int w = it.value("width", 0);
        int h = it.value("height", 0);
        int x = it.value("x", 0);
        int y = it.value("y", 0);
        Furniture* f = createFurnitureFromName(name, w, h, x, y);
        // addFurniture will check bounds/overlap and delete f on failure
        room.addFurniture(f);
    }

    return true;
}

int main() {
    // -------------------------
    // 1) Console Interaction & Room Setup
    // -------------------------
    int roomW = 800, roomH = 600;
    std::cout << "=== 2D Furniture Placement Simulator (Console, improved) ===\n\n";
    std::cout << "Enter room width and height (e.g., 800 600). Press enter for defaults 800 600.\nInput: ";
    
    // Simple check if user pressed enter or typed numbers
    if (std::cin.peek() != '\n') {
        std::cin >> roomW >> roomH;
    }
    // consume rest of line
    std::string dummy; 
    std::getline(std::cin, dummy); 

    Room room(roomW, roomH, false); // allowOverlap=false
    std::cout << "Room created: " << roomW << " x " << roomH << " (collision prevention ON)\n\n";

    std::cout << "Do you want (1) default demo, (2) custom interactive input? Choose 1 or 2: ";
    int choice = 1;
    std::cin >> choice;

    if (choice == 2) {
        int count = 0;
        std::cout << "How many items do you want to add? ";
        std::cin >> count;
        std::cout << "Available types: sofa, table, chair, bed, cupboard, tvstand\n\n";

        for (int i = 0; i < count; ++i) {
            std::string type;
            std::cout << "Item " << (i + 1) << " type: ";
            std::cin >> type;

            // normalize type string if needed, but simple comparison works
            // Default sizes
            int w = 100, h = 100;
            if (type == "sofa") { w=150; h=80; }
            else if (type == "bed") { w=180; h=120; }
            else if (type == "table") { w=100; h=80; }
            else if (type == "tvstand") { w=140; h=40; }
            else if (type == "cupboard") { w=80; h=200; }
            else if (type == "chair") { w=50; h=50; }

            std::cout << "Do you want to enter custom size for this item? (y/N): ";
            char c; std::cin >> c;
            if (c == 'y' || c == 'Y') {
                std::cout << "Enter width (default " << w << "): "; std::cin >> w;
                std::cout << "Enter height (default " << h << "): "; std::cin >> h;
            }

            int x = 0, y = 0;
            std::cout << "Enter x (default 0): "; std::cin >> x;
            std::cout << "Enter y (default 0): "; std::cin >> y;

            // Capitalize first letter for factory
            if (!type.empty()) type[0] = toupper(type[0]);
            
            // Special handling for TVStand/Cupboard casing if needed by factory
            if (type == "Tvstand") type = "TVStand";

            Furniture* f = createFurnitureFromName(type, w, h, x, y);
            if (f) room.addFurniture(f);
            else std::cout << "Unknown furniture type, skipped.\n";
        }
    } else {
        // Default Demo
        room.addFurniture(new Sofa(150,80, 50, 40));
        room.addFurniture(new Bed(180,120, 200, 30));
        room.addFurniture(new Table(100,80, 350, 200));
        room.addFurniture(new TVStand(140,40, 520, 40));
        room.addFurniture(new Cupboard(80,200, 650, 20));
        room.addFurniture(new Chair(50,50, 360, 120));
    }

    // Show ASCII Map
    room.displayAsciiMap(80, 30);
    room.displaySummary();

    // Operator Overload Demos
    std::cout << "\nOperator Overload Demos:\n";
    if (room.count() >= 2) {
        Furniture* a = room.getFurnitureAt(0);
        Furniture* b = room.getFurnitureAt(1);
        std::cout << "Item A: " << *a << " area=" << a->area() << "\n";
        std::cout << "Item B: " << *b << " area=" << b->area() << "\n";
        
        if (a->area() != b->area()) std::cout << " - Areas differ.\n";
        if (*a > *b) std::cout << " - Item A is larger than Item B.\n";
        else std::cout << " - Item A is NOT larger than Item B.\n";

        std::cout << "Shifting Item A by (20,10) using operator+=\n";
        *a += {20, 10};
        std::cout << "Item A now: " << *a << "\n";

        std::cout << "Combined object: ";
        Furniture* combo = a->combineWith(*b);
        combo->showDetails();
        delete combo;
    } else {
        std::cout << "Not enough items for demo.\n";
    }
    std::cout << "\n=== End of demo ===\nLaunching SFML visualization...\n";


    // -------------------------
    // 2) Setup SFML window + scaling
    // -------------------------
    // -------------------------
    // 2) Setup SFML window + scaling
    // -------------------------
    const int windowW = 1000;
    const int windowH = 800;
    sf::RenderWindow window(sf::VideoMode(windowW, windowH), "Furniture Simulator (SFML)");
    window.setFramerateLimit(60);

    // compute uniform scale to fit room inside window
    float scaleX = (float)windowW / (float)room.getWidth();
    float scaleY = (float)windowH / (float)room.getHeight();
    float scale = std::min(scaleX, scaleY);
    if (scale <= 0) scale = 1.0f;

    // -------------------------
    // Load Textures
    // -------------------------
    std::map<std::string, sf::Texture> textures;
    std::vector<std::string> texNames = {"Sofa", "Bed", "Table", "TVStand", "Cupboard", "Chair", "Floor"};
    for (const auto& name : texNames) {
        sf::Texture tex;
        // Try loading from assets folder
        if (tex.loadFromFile("assets/" + name + ".png")) {
            textures[name] = tex;
            textures[name].setSmooth(true);
            std::cout << "Loaded texture: " << name << std::endl;
        } else {
            std::cerr << "Failed to load texture: " << name << std::endl;
        }
    }

    // Setup Floor (Procedural Carpet)
    sf::Sprite floorSprite;
    bool hasFloor = true; // Always true now
    
    // Generate a procedural carpet texture
    sf::Image carpetImg;
    carpetImg.create(64, 64);
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            // Beige base with noise
            int noise = rand() % 20 - 10; // -10 to 10
            int r = std::max(0, std::min(255, 210 + noise));
            int g = std::max(0, std::min(255, 180 + noise));
            int b = std::max(0, std::min(255, 140 + noise));
            carpetImg.setPixel(x, y, sf::Color(r, g, b));
        }
    }
    
    sf::Texture carpetTex;
    carpetTex.loadFromImage(carpetImg);
    carpetTex.setRepeated(true);
    // Store it in map to keep it alive (though we use a local var here, we need it persistent)
    // Actually, we can just keep it in the map
    textures["ProceduralCarpet"] = carpetTex;
    
    floorSprite.setTexture(textures["ProceduralCarpet"]);
    floorSprite.setTextureRect(sf::IntRect(0, 0, windowW, windowH));
    // No tint needed, the texture is already colored
    floorSprite.setColor(sf::Color::White); 


    // -------------------------
    // 3) Create VisualItems from Room
    // -------------------------
    std::vector<VisualItem> visuals;
    auto rebuildVisuals = [&]() {
        visuals.clear();
        for (int i = 0; i < room.count(); ++i) {
            Furniture* f = room.getFurnitureAt(i);
            sf::Texture* t = nullptr;
            if (textures.count(f->getName())) {
                t = &textures[f->getName()];
            }
            visuals.emplace_back(f, scale, t);
        }
    };
    rebuildVisuals();

    // -------------------------
    // 4) Interaction state
    // -------------------------
    bool dragging = false;
    int draggedIndex = -1;
    sf::Vector2f dragOffset{0.f, 0.f};
    bool snapToGrid = true;
    int gridSizeModel = 10; // grid size in model units

    // helper: recalc overlaps visually (AABB)
    auto recalcOverlaps = [&]() {
        for (auto &v : visuals) v.overlapping = false;
        for (size_t i = 0; i < visuals.size(); ++i) {
            for (size_t j = i + 1; j < visuals.size(); ++j) {
                if (visuals[i].shape.getGlobalBounds().intersects(visuals[j].shape.getGlobalBounds())) {
                    visuals[i].overlapping = visuals[j].overlapping = true;
                }
            }
        }
        for (auto &v : visuals) v.updateOutline();
    };

    recalcOverlaps();

    // -------------------------
    // 5) Main event loop
    // -------------------------
    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                window.close();
            }

            // Mouse press -> pick top-most item
            if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mp = window.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
                bool picked = false;
                for (int i = (int)visuals.size() - 1; i >= 0; --i) {
                    if (visuals[i].containsPoint(mp)) {
                        // select only this
                        for (auto &v : visuals) v.selected = false;
                        visuals[i].selected = true;
                        visuals[i].updateOutline();

                        draggedIndex = i;
                        dragging = true;
                        dragOffset = mp - visuals[i].shape.getPosition();
                        picked = true;
                        break;
                    }
                }
                if (!picked) {
                    for (auto &v : visuals) v.selected = false;
                    draggedIndex = -1;
                }
            }

            // Mouse release -> finalize and snap to grid if enabled
            if (ev.type == sf::Event::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
                if (dragging && draggedIndex >= 0 && draggedIndex < (int)visuals.size()) {
                    if (snapToGrid) {
                        // get pixel pos, convert to model units, snap, convert back
                        sf::Vector2f pix = visuals[draggedIndex].shape.getPosition();
                        int modelX = (int)std::round(pix.x / scale);
                        int modelY = (int)std::round(pix.y / scale);
                        modelX = (modelX / gridSizeModel) * gridSizeModel;
                        modelY = (modelY / gridSizeModel) * gridSizeModel;
                        visuals[draggedIndex].shape.setPosition((float)modelX * scale, (float)modelY * scale);
                        visuals[draggedIndex].syncToModel(scale);
                        visuals[draggedIndex].syncFromModel(scale); // Sync sprite position too
                    } else {
                        visuals[draggedIndex].syncToModel(scale);
                        visuals[draggedIndex].syncFromModel(scale); // Sync sprite position too
                    }
                }
                dragging = false;
                draggedIndex = -1;
                recalcOverlaps();
            }

            // Mouse moved -> drag
            if (ev.type == sf::Event::MouseMoved) {
                if (dragging && draggedIndex >= 0) {
                    sf::Vector2f mp = window.mapPixelToCoords({ev.mouseMove.x, ev.mouseMove.y});
                    sf::Vector2f newPos = mp - dragOffset;
                    visuals[draggedIndex].shape.setPosition(newPos);
                    // live-sync for overlap detection and visual feedback
                    visuals[draggedIndex].syncToModel(scale);
                    visuals[draggedIndex].syncFromModel(scale); // Sync sprite position too
                    recalcOverlaps();
                }
            }

            // Keyboard shortcuts
            if (ev.type == sf::Event::KeyPressed) {
                // Toggle snap-to-grid (G)
                if (ev.key.code == sf::Keyboard::G) {
                    snapToGrid = !snapToGrid;
                    std::cout << "Snap-to-grid: " << (snapToGrid ? "ON" : "OFF") << std::endl;
                }

                // Save (S)
                if (ev.key.code == sf::Keyboard::S) {
                    // save to layout.json
                    std::string fname = "layout.json";
                    // ensure models reflect visuals
                    for (auto &v : visuals) v.syncToModel(scale);
                    if (saveLayoutJSON(fname, room)) {
                        std::cout << "Saved layout to " << fname << std::endl;
                    } else {
                        std::cout << "Failed to save layout to " << fname << std::endl;
                    }
                }

                // Load (L)
                if (ev.key.code == sf::Keyboard::L) {
                    std::string fname = "layout.json";
                    if (loadLayoutJSON(fname, room)) {
                        // rebuild visuals from room (we must rebuild because Room owns pointers and was cleared/filled)
                        rebuildVisuals();
                        recalcOverlaps();
                        std::cout << "Loaded layout from " << fname << std::endl;
                    } else {
                        std::cout << "Failed to load layout from " << fname << std::endl;
                    }
                }

                // Delete selected (Del/Backspace)
                if (ev.key.code == sf::Keyboard::Delete || ev.key.code == sf::Keyboard::BackSpace) {
                    int vizIdx = -1;
                    for (size_t i = 0; i < visuals.size(); ++i) if (visuals[i].selected) { vizIdx = (int)i; break; }
                    if (vizIdx >= 0) {
                        Furniture* target = visuals[vizIdx].model;
                        // find room index by pointer
                        int roomIndex = -1;
                        for (int r = 0; r < room.count(); ++r) {
                            if (room.getFurnitureAt(r) == target) { roomIndex = r; break; }
                        }
                        if (roomIndex >= 0) {
                            // remove visual first to avoid dangling pointer exposure
                            visuals.erase(visuals.begin() + vizIdx);
                            // then remove from Room which deletes the pointer
                            room.removeFurniture(roomIndex);
                            recalcOverlaps();
                            std::cout << "Deleted selected item.\n";
                        } else {
                            std::cout << "Could not find selected item in Room.\n";
                        }
                    }
                }

                // Rotate selected (R) - 90 degrees clockwise
                if (ev.key.code == sf::Keyboard::R) {
                    int vizIdx = -1;
                    for (size_t i = 0; i < visuals.size(); ++i) if (visuals[i].selected) { vizIdx = (int)i; break; }
                    if (vizIdx >= 0) {
                        // rotate by swapping width/height in the model
                        Furniture* m = visuals[vizIdx].model;
                        int oldW = m->getWidth();
                        int oldH = m->getHeight();
                        int newW = oldH;
                        int newH = oldW;

                        // Temporarily set new dimensions and update visual
                        m->setDimensions(newW, newH);
                        visuals[vizIdx].syncFromModel(scale);

                        // Check bounds: if it exceeds room bounds after rotation, revert
                        int mx = m->getX();
                        int my = m->getY();
                        bool oob = (mx < 0) || (my < 0) || (mx + newW > room.getWidth()) || (my + newH > room.getHeight());
                        if (oob) {
                            // revert
                            m->setDimensions(oldW, oldH);
                            visuals[vizIdx].syncFromModel(scale);
                            std::cout << "Rotation would exceed room bounds. Rotation cancelled.\n";
                        } else {
                            // success: update visuals and overlap check
                            recalcOverlaps();
                            std::cout << "Rotated selected item by 90 degrees.\n";
                        }
                    }
                }
            }
        } // end event polling

        // -------------------------
        // Render
        // -------------------------
        window.clear(sf::Color(245,245,245));

        // draw floor or grid
        if (hasFloor) {
            window.draw(floorSprite);
        } else {
            const int stepPx = std::max(4, (int)std::round(gridSizeModel * scale));
            sf::VertexArray lines(sf::Lines);
            sf::Color gridCol(230,230,230);
            for (int x = 0; x <= windowW; x += stepPx) {
                lines.append(sf::Vertex(sf::Vector2f((float)x, 0.f), gridCol));
                lines.append(sf::Vertex(sf::Vector2f((float)x, (float)windowH), gridCol));
            }
            for (int y = 0; y <= windowH; y += stepPx) {
                lines.append(sf::Vertex(sf::Vector2f(0.f, (float)y), gridCol));
                lines.append(sf::Vertex(sf::Vector2f((float)windowW, (float)y), gridCol));
            }
            window.draw(lines);
        }

        // draw visuals
        for (auto &v : visuals) {
            v.draw(window);
        }

        window.display();
    }

    return 0;
}
