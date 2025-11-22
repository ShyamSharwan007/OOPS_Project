#include "FurnitureSim.h"
#include <iomanip>
#include <algorithm>
#include <stdexcept> // Make sure it's included here too
using namespace std;
// -----------------------------
// Furniture implementation
// -----------------------------
Furniture::Furniture(string n, int w, int h, int xpos, int ypos, char sym)
    : name(n), width(w), height(h), x(xpos), y(ypos), symbol(sym) {}

Furniture::~Furniture() {}

void Furniture::setPosition(int xpos, int ypos) { x = xpos; y = ypos; }
void Furniture::setDimensions(int w, int h) { width = w; height = h; }

string Furniture::getName() const { return name; }
int Furniture::getWidth() const { return width; }
int Furniture::getHeight() const { return height; }
int Furniture::getX() const { return x; }
int Furniture::getY() const { return y; }
char Furniture::getSymbol() const { return symbol; }

int Furniture::area() const { return width * height; }

bool Furniture::operator==(const Furniture &other) const {
    return this->area() == other.area();
}

bool Furniture::operator>(const Furniture &other) const {
    return this->area() > other.area();
}

Furniture& Furniture::operator+=(const pair<int,int> &shift) {
    this->x += shift.first;
    this->y += shift.second;
    return *this;
}

Furniture* Furniture::combineWith(const Furniture &other) const {
    // Combine names and areas into a SimpleFurniture.
    string newName = this->name + "+" + other.name;
    int newW = this->width + other.width;    // crude combine strategy (width sum)
    int newH = max(this->height, other.height); // height as max
    // create on heap; caller should delete
    return new SimpleFurniture(newName, newW, newH, this->x, this->y, 'X');
}

ostream &operator<<(ostream &out, const Furniture &f) {
    out << f.name << " [" << f.width << "x" << f.height << "] at (" << f.x << "," << f.y << ")";
    return out;
}

// ---------------------------------------
// SimpleFurniture
// ---------------------------------------
SimpleFurniture::SimpleFurniture(string n, int w, int h, int xpos, int ypos, char sym)
    : Furniture(n, w, h, xpos, ypos, sym) {}

void SimpleFurniture::showDetails() const {
    cout << "SimpleFurniture -> " << *this << " area=" << area() << endl;
}

// -----------------------------
// Derived pieces implementation
// -----------------------------
Sofa::Sofa(int w, int h, int xpos, int ypos)
    : Furniture("Sofa", w, h, xpos, ypos, 'S') {}

void Sofa::showDetails() const {
    cout << "Sofa -> " << *this << " area=" << area() << endl;
}

Table::Table(int w, int h, int xpos, int ypos)
    : Furniture("Table", w, h, xpos, ypos, 'T') {}

void Table::showDetails() const {
    cout << "Table -> " << *this << " area=" << area() << endl;
}

Chair::Chair(int w, int h, int xpos, int ypos)
    : Furniture("Chair", w, h, xpos, ypos, 'C') {}

void Chair::showDetails() const {
    cout << "Chair -> " << *this << " area=" << area() << endl;
}

Bed::Bed(int w, int h, int xpos, int ypos)
    : Furniture("Bed", w, h, xpos, ypos, 'B') {}

void Bed::showDetails() const {
    cout << "Bed -> " << *this << " area=" << area() << endl;
}

Cupboard::Cupboard(int w, int h, int xpos, int ypos)
    : Furniture("Cupboard", w, h, xpos, ypos, 'U') {} // U for cUpboard

void Cupboard::showDetails() const {
    cout << "Cupboard -> " << *this << " area=" << area() << endl;
}

TVStand::TVStand(int w, int h, int xpos, int ypos)
    : Furniture("TVStand", w, h, xpos, ypos, 'V') {} // V for tVstand

void TVStand::showDetails() const {
    cout << "TVStand -> " << *this << " area=" << area() << endl;
}

// -----------------------------
// Room implementation
// -----------------------------
Room::Room(int w, int h, bool allowOverlap) : width(w), height(h) , allowOverlap(allowOverlap) {}

Room::~Room() {
    for (auto p : furnitureList) delete p;
    furnitureList.clear();
}

int Room::getWidth() const { return width; }
int Room::getHeight() const { return height; }

bool Room::addFurniture(Furniture *f) {
    // boundary check
    if (f->getX() < 0 || f->getY() < 0) {
        cout << " " << f->getName() << " cannot be placed at negative coordinates!\n";
        delete f; // Still delete the failed object
        // Throw an exception
        throw std::out_of_range(f->getName() + " placed at negative coordinates.");
    }
    if (f->getX() + f->getWidth() > width || f->getY() + f->getHeight() > height) {
        cout << " " << f->getName() << " out of room bounds! Not added.\n";
        delete f;
        // Throw an exception
        throw out_of_range(f->getName() + " placed out of room bounds.");
    }
    
    // we allow overlap for "realistic, uneven" placements (but report overlap warning)
    // check overlap with existing furniture (optional warning only)
    for (auto other : furnitureList) {
        int ax1 = f->getX(), ay1 = f->getY();
        int ax2 = ax1 + f->getWidth(), ay2 = ay1 + f->getHeight();
        int bx1 = other->getX(), by1 = other->getY();
        
        // *** THIS IS THE CORRECTED LINE ***
        int bx2 = bx1 + other->getWidth(), by2 = by1 + other->getHeight();
        
        bool overlap = !(ax2 <= bx1 || ax1 >= bx2 || ay2 <= by1 || ay1 >= by2);
        if (overlap) {
            cout << "⚠ Warning: " << f->getName() << " overlaps with " << other->getName() << endl;
            break; // just a warning
        }
    }

    furnitureList.push_back(f);
    cout << "Added " << f->getName() << " at (" << f->getX() << "," << f->getY() << ")\n";
    return true;
}

void Room::removeFurniture(int index) {
    if (index < 0 || index >= (int)furnitureList.size()) {
        cout << " Invalid index to remove.\n";
        return;
    }
    delete furnitureList[index];
    furnitureList.erase(furnitureList.begin() + index);
    cout << "Removed furniture at index " << index << endl;
}

void Room::displayAll() const {
    cout << "\nRoom (" << width << "x" << height << ") contains " << furnitureList.size() << " items:\n";
    for (size_t i = 0; i < furnitureList.size(); ++i) {
        cout << setw(2) << i << ". ";
        furnitureList[i]->showDetails();
    }
}

int Room::totalOccupiedArea() const {
    int s = 0;
    for (auto p : furnitureList) s += p->area();
    return s;
}

int Room::totalRoomArea() const {
    return width * height;
}

void Room::displaySummary() const {
    int occ = totalOccupiedArea();
    int tot = totalRoomArea();
    cout << "\nSummary:\n";
    cout << " Total room area: " << tot << "\n";
    cout << " Occupied area (sum of furniture areas): " << occ << "\n";
    double perc = tot == 0 ? 0.0 : (100.0 * occ / tot);
    cout << " Occupied percent (sum): " << fixed << setprecision(2) << perc << "%\n";
    cout << " Free area (approx): " << (tot - occ) << "\n";
}

// ASCII scaled map
void Room::displayAsciiMap(int cols, int rows) const {
    if (cols < 10) cols = 10;
    if (rows < 6) rows = 6;

    // scale factors: how many room units per grid cell
    double xScale = (double)width / cols;
    double yScale = (double)height / rows;

    // initialize grid with dots
    vector<string> grid(rows, string(cols, '.'));

    // fill grid by furniture shapes (simple occupancy mapping)
    for (auto p : furnitureList) {
        int left = (int)floor(p->getX() / xScale);
        int top = (int)floor(p->getY() / yScale);
        int right = (int)ceil((p->getX() + p->getWidth()) / xScale);
        int bottom = (int)ceil((p->getY() + p->getHeight()) / yScale);

        left = max(0, min(left, cols-1));
        right = max(0, min(right, cols));
        top = max(0, min(top, rows-1));
        bottom = max(0, min(bottom, rows));

        for (int r = top; r < bottom; ++r) {
            for (int c = left; c < right; ++c) {
                grid[r][c] = p->getSymbol();
            }
        }
    }

    // Print top border
    cout << "\nAscii Map (scaled to " << cols << "x" << rows << ")\n";
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) cout << grid[r][c];
        cout << '\n';
    }
}

int Room::count() const { return (int)furnitureList.size(); }
Furniture* Room::getFurnitureAt(int idx) const {
    if (idx < 0 || idx >= (int)furnitureList.size()) return nullptr;
    return furnitureList[idx];
}