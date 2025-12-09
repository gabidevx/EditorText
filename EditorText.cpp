#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Clipboard.hpp> // clipboard
#include <optional>

#include <vector> // vectori
#include <string> //strin uri
#include <algorithm> // min, max

using namespace std;

enum source {
    ORIGINAL,
    ADD
};

struct piece {
    source source;
    int start;
    int length;
};

struct editor {
    string originalBuffer;
    string addBuffer;

    int cursorPosition;
    int selectionAnchor;

    vector <piece> pieces;
    vector <int> lineStarts;
};

void updateLineStarts(editor& ed);
string buildText(const editor& ed);
void deleteCharBeforeCursor(editor& ed);


bool hasSelection(const editor& ed) {
    return ed.cursorPosition != ed.selectionAnchor;
}

string getSelectedText(const editor& ed) {
    if (!hasSelection(ed)) return "";

    string fullText = buildText(ed);
    int start = min(ed.cursorPosition, ed.selectionAnchor);
    int end = max(ed.cursorPosition, ed.selectionAnchor);

    if (start < 0) start = 0;
    if (end > fullText.length()) end = fullText.length();

    return fullText.substr(start, end - start);
}

void deleteSelection(editor& ed) {
    if (!hasSelection(ed)) return;

    int start = min(ed.cursorPosition, ed.selectionAnchor);
    int end = max(ed.cursorPosition, ed.selectionAnchor);

    //se muta cursorul la final
    ed.cursorPosition = end;

    //stergere caracter cu caracter
    int count = end - start;
    for (int i = 0; i < count; i++) {
        deleteCharBeforeCursor(ed);
    }

    //resetare selectie
    ed.selectionAnchor = ed.cursorPosition;
}

void initEditor(editor& ed, string startText) {
    ed.originalBuffer = startText;
    ed.addBuffer = "";
    ed.pieces.clear();

    piece p = { ORIGINAL, 0, (int)startText.length() };
    ed.pieces.push_back(p);

    ed.cursorPosition = startText.length();
    ed.selectionAnchor = ed.cursorPosition;
    updateLineStarts(ed);
}

void insertTextAtCursor(editor& ed, string text) {
    int startPosInBuffer = ed.addBuffer.length();
    ed.addBuffer += text;

    piece newPiece = { ADD, startPosInBuffer, (int)text.length() };

    int currentLen = 0;
    int index = -1;
    int offsetInPiece = 0;

    for (int i = 0; i < ed.pieces.size(); i++) {
        if (ed.cursorPosition >= currentLen && ed.cursorPosition <= currentLen + ed.pieces[i].length) {
            index = i;
            offsetInPiece = ed.cursorPosition - currentLen;
            break;
        }
        currentLen += ed.pieces[i].length;
    }

    if (index == -1) {
        ed.pieces.push_back(newPiece);
        ed.cursorPosition += text.length();
        updateLineStarts(ed);
        return;
    }


    piece oldPiece = ed.pieces[index];
    vector<piece> replacements;

    if (offsetInPiece > 0) {
        piece left = oldPiece;
        left.length = offsetInPiece;
        replacements.push_back(left);
    }

    replacements.push_back(newPiece);

    if (offsetInPiece < oldPiece.length) {
        piece right = oldPiece;
        right.start += offsetInPiece;
        right.length -= offsetInPiece;
        replacements.push_back(right);
    }

    ed.pieces.erase(ed.pieces.begin() + index);
    ed.pieces.insert(ed.pieces.begin() + index, replacements.begin(), replacements.end());

    ed.cursorPosition += text.length();
    updateLineStarts(ed);
}

void deleteCharBeforeCursor(editor& ed) {
    if (ed.cursorPosition == 0) return;

    int deletePos = ed.cursorPosition - 1;
    int currentLen = 0;
    int index = -1;
    int offsetInPiece = 0;

    for (int i = 0; i < ed.pieces.size(); i++) {
        if (deletePos >= currentLen && deletePos < currentLen + ed.pieces[i].length) {
            index = i;
            offsetInPiece = deletePos - currentLen;
            break;
        }
        currentLen += ed.pieces[i].length;
    }

    if (index == -1) return;

    piece oldPiece = ed.pieces[index];
    vector <piece> replacements;

    if (offsetInPiece > 0) {
        piece left = oldPiece;
        left.length = offsetInPiece;
        replacements.push_back(left);
    }

    if (offsetInPiece + 1 < oldPiece.length) {
        piece right = oldPiece;
        right.start += (offsetInPiece + 1);
        right.length -= (offsetInPiece + 1);
        replacements.push_back(right);
    }

    ed.pieces.erase(ed.pieces.begin() + index);
    if (!replacements.empty()) {
        ed.pieces.insert(ed.pieces.begin() + index, replacements.begin(), replacements.end());
    }

    ed.cursorPosition--;
    updateLineStarts(ed);
}

string buildText(const editor& ed) {
    string fullText = "";
    for (const auto& p : ed.pieces) {
        if (p.source == ORIGINAL) fullText += ed.originalBuffer.substr(p.start, p.length);
        else fullText += ed.addBuffer.substr(p.start, p.length);
    }
    return fullText;
}

void updateLineStarts(editor& ed) {
    ed.lineStarts.clear();
    ed.lineStarts.push_back(0);
    string text = buildText(ed);
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == '\n') ed.lineStarts.push_back(i + 1);
    }
}

void initWindow(editor& notepad) {
    sf::RenderWindow window(sf::VideoMode({ 1000, 700 }), "Notepad");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        cout << "Eroare: Nu am gasit arial.ttf!" << endl;
        return;
    }

    sf::Text text(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::Black);

    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            //resize la window
            else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2f newSize(window.getSize());
                sf::View newView(sf::FloatRect({ 0.f, 0.f }, newSize));
                window.setView(newView);
            }

            //textul introdus
            else if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                char c = static_cast<char>(textEvent->unicode);

                //evitarea dubluapasare ctrl
                bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
                if (ctrlPressed) continue;

                //caractere care nu pot fi scrise
                if (c < 32 && c != 8 && c != 13) continue;

                if (c == 8) { //sterge
                    if (hasSelection(notepad)) {
                        deleteSelection(notepad); //sterge selectia
                    }
                    else {
                        deleteCharBeforeCursor(notepad); //sterge o litera
                    }
                }
                else {// enter/litere
                    //daca e ceva selectat, stergem
                    if (hasSelection(notepad)) {
                        deleteSelection(notepad);
                    }

                    if (c == 13) insertTextAtCursor(notepad, "\n");
                    else insertTextAtCursor(notepad, string(1, c));
                }

                //resetare ancora dupa ce scriem
                notepad.selectionAnchor = notepad.cursorPosition;
            }

            //daca se apasa orice key
            else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
                bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

                if (keyEvent->code == sf::Keyboard::Key::Left) {
                    if (notepad.cursorPosition > 0) notepad.cursorPosition--;
                    if (!shift) notepad.selectionAnchor = notepad.cursorPosition;
                }
                else if (keyEvent->code == sf::Keyboard::Key::Right) {
                    if (notepad.cursorPosition < buildText(notepad).length()) notepad.cursorPosition++;
                    if (!shift) notepad.selectionAnchor = notepad.cursorPosition;
                }
                // CTRL + A
                else if (keyEvent->code == sf::Keyboard::Key::A && ctrl) {
                    notepad.selectionAnchor = 0;
                    notepad.cursorPosition = buildText(notepad).length();
                }
                // CTRL + C
                else if (keyEvent->code == sf::Keyboard::Key::C && ctrl) {
                    if (hasSelection(notepad)) {
                        sf::Clipboard::setString(getSelectedText(notepad));
                    }
                }
                // CTRL + V
                else if (keyEvent->code == sf::Keyboard::Key::V && ctrl) {
                    //stergem daca e ceva selectat
                    if (hasSelection(notepad)) {
                        deleteSelection(notepad);
                    }
                    //inseram textul in notepad
                    string clip = sf::Clipboard::getString();
                    insertTextAtCursor(notepad, clip);

                    //resetam unde sta inceputul selectiei
                    notepad.selectionAnchor = notepad.cursorPosition;
                }
            }
        }

        //notepad ul
        window.clear(sf::Color::White);

        //text
        text.setString(buildText(notepad));
        text.setPosition({ 60, 0 });

        //marginea gri stanga
        float winH = window.getView().getSize().y;
        sf::RectangleShape gutter(sf::Vector2f({ 50, winH }));
        gutter.setFillColor(sf::Color(230, 230, 230));
        window.draw(gutter);

        //numere linii
        sf::Text lineNum = text;
        lineNum.setFillColor(sf::Color::Black);
        float lineH = font.getLineSpacing(24);
        for (int i = 0; i < notepad.lineStarts.size(); i++) {
            lineNum.setString(to_string(i + 1));
            lineNum.setPosition({ 5, i * lineH });
            window.draw(lineNum);
        }

        if (hasSelection(notepad)) {
            int start = min(notepad.cursorPosition, notepad.selectionAnchor);
            int end = max(notepad.cursorPosition, notepad.selectionAnchor);

            sf::Vector2f p1 = text.findCharacterPos(start);
            sf::Vector2f p2 = text.findCharacterPos(end);

            //desenare selectie
            sf::RectangleShape selRect;
            selRect.setPosition(p1);

            float w = p2.x - p1.x;
            if (p2.y > p1.y) w = 15; // nu e implementat pentru mai multe linii

            selRect.setSize({ w, lineH });
            selRect.setFillColor(sf::Color(100, 150, 255, 128));//culoare selectie
            window.draw(selRect);
        }

        //text
        window.draw(text);

        //curosor
        sf::Vector2f curPos = text.findCharacterPos(notepad.cursorPosition);
        sf::RectangleShape curLine(sf::Vector2f({ 2, 24 }));
        curLine.setFillColor(sf::Color::Red);
        curLine.setPosition(curPos);
        window.draw(curLine);

        window.display();
    }
}

int main() {
    editor notepad;
    initEditor(notepad, "...");
    initWindow(notepad);
    return 0;
}