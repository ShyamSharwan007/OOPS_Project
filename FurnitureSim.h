#ifndef FURNITURESIM_H
#define FURNITURESIM_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include <stdexcept> // For exception handling
using namespace std;

// -----------------------------
// Abstract Base Class: Furniture
// -----------------------------
class Furniture {
protected:
    string name;
    int width, height;   // in "room units" (same as room dims)
    int x, y;            // top-left position in room coords
    char symbol;         // ASCII symbol for grid visualization

public:
    Furniture(string n = "Unknown", int w = 0, int h = 0, int xpos = 0, int ypos = 0, char sym = '?');
    virtual ~Furniture();

    // Encapsulation
    void setPosition(int xpos, int ypos);
    void setDimensions(int w, int h);

    string getName() const;
    int getWidth() const;
    int getHeight() const;
    int getX() const;
    int getY() const;
    char getSymbol() const;

    int area() const; // convenience

    // Abstraction
    virtual void showDetails() const = 0;

    // Operator Overloads
    bool operator==(const Furniture &other) const; // by area
    bool operator>(const Furniture &other) const;  // by area
    
    // Shift position
    Furniture& operator+=(const pair<int,int> &shift);

    // Combine two pieces into a new simple furniture (merged name & area)
    // returns a concrete SimpleFurniture (declared later)
    // signature here returns pointer to heap object; caller must manage/don't forget to delete
    Furniture* combineWith(const Furniture &other) const;

    // Stream operator
    friend ostream &operator<<(ostream &out, const Furniture &f);
};

// ---------------------------------------
// Concrete SimpleFurniture (for combos)
// ---------------------------------------
class SimpleFurniture : public Furniture {
public:
    SimpleFurniture(string n, int w, int h, int xpos = 0, int ypos = 0, char sym = 'C');
    void showDetails() const override;
};

// -----------------------------
// Derived Classes
// -----------------------------
class Sofa : public Furniture {
public:
    Sofa(int w, int h, int xpos, int ypos);
    void showDetails() const override;
};

class Table : public Furniture {
public:
    Table(int w, int h, int xpos, int ypos);
    void showDetails() const override;
};

class Chair : public Furniture {
public:
    Chair(int w, int h, int xpos, int ypos);
    void showDetails() const override;
};

class Bed : public Furniture {
public:
    Bed(int w, int h, int xpos, int ypos);
    void showDetails() const override;
};

class Cupboard : public Furniture {
public:
    Cupboard(int w, int h, int xpos, int ypos);
    void showDetails() const override;
};

class TVStand : public Furniture {
public:
    TVStand(int w, int h, int xpos, int ypos);
    void showDetails() const override;
};

// -----------------------------
// Composition: Room
// -----------------------------
class Room {
private:
    int width, height;                // room dimensions in same units
    vector<Furniture*> furnitureList; // owns pointers
    bool allowOverlap;

public:
    Room(int w = 800, int h = 600, bool allowOverlap = true);
    ~Room();

    int getWidth() const;
    int getHeight() const;

    bool addFurniture(Furniture *f);      // returns true if added
    void removeFurniture(int index);      // remove by index (0-based)
    void displayAll() const;              // textual details
    void displaySummary() const;          // occupied/available area
    void displayAsciiMap(int cols = 60, int rows = 20) const; // scaled grid
    int totalOccupiedArea() const;
    int totalRoomArea() const;
    int count() const;
    Furniture* getFurnitureAt(int idx) const;

    // Template function to find the first item of a specific class
    // (Defined in header as it's a template)
    template<typename T>
    T* findFirstItemOfType() const {
        for (Furniture* f : furnitureList) {
            // dynamic_cast checks if 'f' can be safely cast to a 'T*'
            T* specificItem = dynamic_cast<T*>(f);
            if (specificItem != nullptr) {
                return specificItem; // Found one!
            }
        }
        return nullptr; // Found none
    }
};

#endif