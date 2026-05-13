#include "main.h"

const float PI = 3.14159265358979323846f;

struct DisplayData {
	float width_mm = 0, height_mm = 0, diagonal_mm = 0;
};

// 입력 박스 구조체
struct InputField {
	sf::RectangleShape box;
	sf::Text text;
	std::string content;
	bool isFocused = false;
	bool isReadOnly = false;
	int row, col;
};

// 매트릭스 초기 텍스트
std::string rowLabels[] = { "Pixels(px)", "Ratio/Constant", "Millimeters (mm)", "Inches (in)", "Pixel Pitch" };
std::string colLabels[] = { "Metric", "Width", "Height", "Diagonal" };

// 텍스트 레이블 생성 및 회전 설정 함수
void setupLabel(sf::Text& text, const std::string& content, sf::Vector2f pos, float rotation) {
	text.setString(content);
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
	text.setPosition(pos);
	text.setRotation(rotation);
}

// diagonal 값이 입력되는 경우에는 가로 및 세로 중 가장 긴 길이를 고정으로 두고 나머지를 조정하도록 추가 분기
// 헤더 출력 오류 수정
// ratio 갱신 오류 수정
int main() {
    sf::RenderWindow window(sf::VideoMode(630, 700), "GeoDisplay Calculator");
    sf::Font font;
    if (!font.loadFromFile("c:/windows/fonts/arial.ttf")) {
        std::cout << "Font load failed!" << std::endl;
        return -1;
    }

    std::vector<InputField> fields;
    DisplayData displayData = { 0, 0, 0 };

    // 1. 필드 초기화 및 초기 텍스트 설정 (오류 1 수정: 초기 문자열 설정 보강)
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 4; ++c) {
            InputField field;
            field.row = r; field.col = c;
            if (r == 0) { field.content = colLabels[c]; field.isReadOnly = true; }
            else if (c == 0) { field.content = rowLabels[r - 1]; field.isReadOnly = true; }
            else {
                field.content = "";
                field.isReadOnly = (r == 5 || (r == 1 && c == 3));
            }
            field.box.setSize({ 110.0f, 30.0f });
            field.box.setPosition(30.0f + c * 115.0f, 50.0f + r * 35.0f);
            field.box.setFillColor(field.isReadOnly ? sf::Color(70, 70, 70) : sf::Color(45, 45, 45));
            field.box.setOutlineThickness(1);
            field.box.setOutlineColor(sf::Color(120, 120, 120));

            field.text.setFont(font);
            field.text.setCharacterSize(13);
            field.text.setString(field.content); // 초기 레이블 즉시 반영
            field.text.setPosition(field.box.getPosition().x + 5, field.box.getPosition().y + 5);
            fields.push_back(field);
        }
    }

    auto getField = [&](int r, int c) -> InputField& { return fields[r * 4 + c]; };

    // 2. 통합 계산 로직
    auto updateAllCalculations = [&](InputField& f) {
        try {
            // 직각삼각형 계산 함수 : 대각선 입력 시 로직 분기
            auto solveTriangle = [&](int row, int triggeredCol) {
                float w = getField(row, 1).content.empty() ? 0 : std::stof(getField(row, 1).content);
                float h = getField(row, 2).content.empty() ? 0 : std::stof(getField(row, 2).content);
                float d = getField(row, 3).content.empty() ? 0 : std::stof(getField(row, 3).content);

                if (triggeredCol == 3 && d > 0) { // 대각선이 입력된 경우 
                    if (w > 0 || h > 0) {
                        float currentMax = std::max(w, h);
                        if (w >= h) { w = currentMax; h = std::sqrt(std::max(0.0f, d * d - w * w)); }
                        else { h = currentMax; w = std::sqrt(std::max(0.0f, d * d - h * h)); }
                    }
                }
                else { // 가로/세로가 입력된 경우 기본 피타고라스
                    if (w > 0 && h > 0) d = std::sqrt(w * w + h * h);
                    else if (w > 0 && d > 0) h = std::sqrt(std::max(0.0f, d * d - w * w));
                    else if (h > 0 && d > 0) w = std::sqrt(std::max(0.0f, d * d - h * h));
                }

                if (w > 0)
                    getField(row, 1).content = std::to_string(w).substr(0, 7);
                if (h > 0)
                    getField(row, 2).content = std::to_string(h).substr(0, 7);
                if (d > 0)
                    getField(row, 3).content = std::to_string(d).substr(0, 7);
                };

            if (f.row == 3 || f.row == 4) { // mm 또는 inch 입력 시
                solveTriangle(f.row, f.col);
                int targetRow = (f.row == 3) ? 4 : 3;
                float factor = (f.row == 3) ? (1.0f / 25.4f) : 25.4f;

                for (int c = 1; c <= 3; ++c) {
                    float val = std::stof(getField(f.row, c).content);
                    getField(targetRow, c).content = std::to_string(val * factor).substr(0, 7);
                }

                // Ratio 자동 업데이트 로직 강화
                float mw = std::stof(getField(3, 1).content);
                float mh = std::stof(getField(3, 2).content);
                if (mw > 0 && mh > 0) {
                    float base = std::min(mw, mh);
                    getField(2, 1).content = std::to_string(mw / base).substr(0, 6);
                    getField(2, 2).content = std::to_string(mh / base).substr(0, 6);
                }
            }
            else if (f.row == 2) { // Ratio 수정 시
                solveTriangle(2, f.col);
                float rw = std::stof(getField(2, 1).content), rh = std::stof(getField(2, 2).content);
                float mw = getField(3, 1).content.empty() ? 0 : std::stof(getField(3, 1).content);
                float mh = getField(3, 2).content.empty() ? 0 : std::stof(getField(3, 2).content);

                if (mw > 0 || mh > 0) {
                    float currentMax = std::max(mw, mh);
                    if (rw >= rh) { mw = currentMax; mh = currentMax * (rh / rw); }
                    else { mh = currentMax; mw = currentMax * (rw / rh); }
                    getField(3, 1).content = std::to_string(mw);
                    getField(3, 2).content = std::to_string(mh);
                    solveTriangle(3, 0); // mm 삼각형 최종 완성
                    for (int c = 1; c <= 3; ++c)
                        getField(4, c).content = std::to_string(std::stof(getField(3, c).content) / 25.4f).substr(0, 7);
                }
            }

            // 픽셀 피치 및 렌더링 데이터 갱신
            float pw = getField(1, 1).content.empty() ? 0 : std::stof(getField(1, 1).content);
            float ph = getField(1, 2).content.empty() ? 0 : std::stof(getField(1, 2).content);
            if (pw > 0 && ph > 0 && !getField(3, 3).content.empty()) {
                float md = std::stof(getField(3, 3).content);
                getField(5, 1).content = std::to_string(md / std::sqrt(pw * pw + ph * ph)).substr(0, 8);
            }

            if (!getField(3, 1).content.empty()) {
                displayData.width_mm = std::stof(getField(3, 1).content);
                displayData.height_mm = std::stof(getField(3, 2).content);
                displayData.diagonal_mm = std::stof(getField(3, 3).content);
            }

            for (auto& field : fields) field.text.setString(field.content);
        }
        catch (...) {}
        };

    // 3. 메인 루프 (이벤트 처리 및 렌더링)
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);
                for (auto& f : fields) {
                    if (f.isFocused && !f.box.getGlobalBounds().contains(mousePos)) updateAllCalculations(f);
                    if (!f.isReadOnly && f.box.getGlobalBounds().contains(mousePos)) {
                        f.isFocused = true; f.box.setOutlineColor(sf::Color::Yellow);
                    }
                    else {
                        f.isFocused = false; f.box.setOutlineColor(sf::Color(120, 120, 120));
                    }
                }
            }

            if (event.type == sf::Event::TextEntered) {
                for (auto& f : fields) {
                    if (!f.isFocused || f.isReadOnly) continue;
                    if (event.text.unicode == 13 || event.text.unicode == 9) {
                        updateAllCalculations(f);
                        f.isFocused = false; f.box.setOutlineColor(sf::Color(120, 120, 120));

                        // 포커스 이동 로직
                        int nextIdx = (f.row * 4 + f.col + 1) % fields.size();
                        while (fields[nextIdx].isReadOnly || fields[nextIdx].col == 0) nextIdx = (nextIdx + 1) % fields.size();
                        fields[nextIdx].isFocused = true; fields[nextIdx].box.setOutlineColor(sf::Color::Yellow);
                        break;
                    }
                    else if (event.text.unicode == 8 && !f.content.empty()) f.content.pop_back();
                    else if (event.text.unicode < 128 && event.text.unicode >= 32) f.content += static_cast<char>(event.text.unicode);
                    f.text.setString(f.content);
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));
        for (auto& f : fields) { window.draw(f.box); window.draw(f.text); }

        if (displayData.width_mm > 0 && displayData.height_mm > 0) {
            float scale = 250.0f / std::max(displayData.width_mm, displayData.height_mm);
            float offsetX = 20.0f, offsetY = 340.0f;

            // --- 사각형(Rectangle) 그리기 ---
            sf::RectangleShape rect(sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
            rect.setPosition(offsetX + 150.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
            rect.setFillColor(sf::Color::Transparent);
            rect.setOutlineThickness(2);
            rect.setOutlineColor(sf::Color::White);
            window.draw(rect);

            // --- 삼각형(Triangle) 그리기 ---
            sf::ConvexShape tri(3);
            tri.setPoint(0, sf::Vector2f(0, displayData.height_mm * scale));
            tri.setPoint(1, sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
            tri.setPoint(2, sf::Vector2f(0, 0));
            tri.setPosition(offsetX + 450.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
            tri.setOutlineThickness(2);
            tri.setOutlineColor(sf::Color::Cyan);
            window.draw(tri);

            // --- 레이블 렌더링 설정 ---
            sf::Text label;
            label.setFont(font);
            label.setCharacterSize(12);
            label.setFillColor(sf::Color::White);

            // 1. 사각형 레이블 (가로, 세로)
            setupLabel(label, std::to_string((int)displayData.width_mm) + "mm",
                { rect.getPosition().x + rect.getSize().x / 2.0f, rect.getPosition().y + rect.getSize().y + 15.0f }, 0);
            window.draw(label);

            setupLabel(label, std::to_string((int)displayData.height_mm) + "mm",
                { rect.getPosition().x - 15.0f, rect.getPosition().y + rect.getSize().y / 2.0f }, -90);
            window.draw(label);

            // 2. 삼각형 레이블 (가로, 세로, 빗변)
            // 삼각형 가로 (하단)
            setupLabel(label, std::to_string((int)displayData.width_mm) + "mm",
                { tri.getPosition().x + (displayData.width_mm * scale) / 2.0f, tri.getPosition().y + (displayData.height_mm * scale) + 15.0f }, 0);
            window.draw(label);

            // 삼각형 세로 (좌측)
            setupLabel(label, std::to_string((int)displayData.height_mm) + "mm",
                { tri.getPosition().x - 15.0f, tri.getPosition().y + (displayData.height_mm * scale) / 2.0f }, -90);
            window.draw(label);

            // 삼각형 빗변 (기울기 보정 및 위치 조정)
            // [보정추가] atan2는 라디안 값을 반환 -> SFML의 회전 방향에 맞춰 각도를 계산해서 반영하기
            // 기존 -angle에서 반대 방향인 angle로 보정하여 기울기와 평행하게 맞춰야 함
            float angleRad = std::atan2(displayData.height_mm, displayData.width_mm);
            float angleDeg = angleRad * 180.0f / 3.141592f;

            // 텍스트 위치: 빗변의 중앙 지점
            float midX = tri.getPosition().x + (displayData.width_mm * scale) / 2.0f;
            float midY = tri.getPosition().y + (displayData.height_mm * scale) / 2.0f;
                     
            setupLabel(label, std::to_string((int)displayData.diagonal_mm) + "mm",
                { midX, midY - 15.0f }, angleDeg);
            window.draw(label);
        }

        window.display();
    }
    return 0;
}

// 오류 있음 
// 1. 최초 값을 작성하기 전까지 헤더 텍스트들이 출력되지 않는 오류
// 2. mm 또는 inch 작성 시 세 변의 값이 구해졌는데도 ratio 가 이에 맞추어 업데이트 되지 않는 오류
//int main() {
//    sf::RenderWindow window(sf::VideoMode(1150, 450), "GeoDisplay Calculator");
//    sf::Font font;
//    if (!font.loadFromFile("c:/windows/fonts/arial.ttf")) {
//        std::cout << "Font load failed!" << std::endl;
//        return -1;
//    }
//
//    std::vector<InputField> fields;
//    DisplayData displayData = { 0, 0, 0 };
//
//    // 1. 필드 초기화 
//    for (int r = 0; r < 6; ++r) {
//        for (int c = 0; c < 4; ++c) {
//            InputField field;
//            field.row = r; field.col = c;
//            if (r == 0) { field.content = colLabels[c]; field.isReadOnly = true; }
//            else if (c == 0) { field.content = rowLabels[r - 1]; field.isReadOnly = true; }
//            else {
//                field.content = "";
//                field.isReadOnly = (r == 5 || (r == 1 && c == 3)); // 6행 및 2행 4열 잠금
//            }
//            field.box.setSize({ 110.0f, 30.0f });
//            field.box.setPosition(30.0f + c * 115.0f, 50.0f + r * 35.0f);
//            field.box.setFillColor(field.isReadOnly ? sf::Color(70, 70, 70) : sf::Color(45, 45, 45));
//            field.box.setOutlineThickness(1);
//            field.box.setOutlineColor(sf::Color(120, 120, 120));
//            field.text.setFont(font);
//            field.text.setCharacterSize(13);
//            field.text.setPosition(field.box.getPosition().x + 5, field.box.getPosition().y + 5);
//            fields.push_back(field);
//        }
//    }
//
//    auto getField = [&](int r, int c) -> InputField& { return fields[r * 4 + c]; };
//
//    // 2. 데이터 업데이트 핵심 로직
//    auto updateAllCalculations = [&](InputField& f) {
//        try {
//            auto solveTriangle = [&](int row) {
//                float w = getField(row, 1).content.empty() ? 0 : std::stof(getField(row, 1).content);
//                float h = getField(row, 2).content.empty() ? 0 : std::stof(getField(row, 2).content);
//                float d = getField(row, 3).content.empty() ? 0 : std::stof(getField(row, 3).content);
//                if (w > 0 && h > 0) d = std::sqrt(w * w + h * h);
//                else if (w > 0 && d > 0) h = std::sqrt(std::max(0.0f, d * d - w * w));
//                else if (h > 0 && d > 0) w = std::sqrt(std::max(0.0f, d * d - h * h));
//                else return;
//                getField(row, 1).content = std::to_string(w).substr(0, 7);
//                getField(row, 2).content = std::to_string(h).substr(0, 7);
//                getField(row, 3).content = std::to_string(d).substr(0, 7);
//                };
//
//            if (f.row == 3 || f.row == 4) {
//                solveTriangle(f.row);
//                int targetRow = (f.row == 3) ? 4 : 3;
//                float factor = (f.row == 3) ? (1.0f / 25.4f) : 25.4f;
//                for (int c = 1; c <= 3; ++c) {
//                    float val = std::stof(getField(f.row, c).content);
//                    getField(targetRow, c).content = std::to_string(val * factor).substr(0, 7);
//                }
//            }
//            else if (f.row == 2) {
//                solveTriangle(2);
//                float rw = std::stof(getField(2, 1).content), rh = std::stof(getField(2, 2).content);
//                float mw = getField(3, 1).content.empty() ? 0 : std::stof(getField(3, 1).content);
//                float mh = getField(3, 2).content.empty() ? 0 : std::stof(getField(3, 2).content);
//                if (mw > 0 || mh > 0) {
//                    float currentMax = std::max(mw, mh);
//                    if (rw >= rh) { mw = currentMax; mh = currentMax * (rh / rw); }
//                    else { mh = currentMax; mw = currentMax * (rw / rh); }
//                    getField(3, 1).content = std::to_string(mw); getField(3, 2).content = std::to_string(mh);
//                    solveTriangle(3);
//                    for (int c = 1; c <= 3; ++c) getField(4, c).content = std::to_string(std::stof(getField(3, c).content) / 25.4f).substr(0, 7);
//                }
//            }
//
//            // 픽셀 피치 계산
//            float pw = getField(1, 1).content.empty() ? 0 : std::stof(getField(1, 1).content);
//            float ph = getField(1, 2).content.empty() ? 0 : std::stof(getField(1, 2).content);
//            if (pw > 0 && ph > 0 && !getField(3, 3).content.empty()) {
//                float md = std::stof(getField(3, 3).content);
//                getField(5, 1).content = std::to_string(md / std::sqrt(pw * pw + ph * ph)).substr(0, 8);
//            }
//
//            // [중요] 렌더링 데이터 안전하게 업데이트
//            if (!getField(3, 1).content.empty() && !getField(3, 2).content.empty()) {
//                displayData.width_mm = std::stof(getField(3, 1).content);
//                displayData.height_mm = std::stof(getField(3, 2).content);
//                displayData.diagonal_mm = getField(3, 3).content.empty() ? 0 : std::stof(getField(3, 3).content);
//            }
//
//            for (auto& field : fields) field.text.setString(field.content);
//        }
//        catch (...) {}
//        };
//
//    while (window.isOpen()) {
//        sf::Event event;
//        while (window.pollEvent(event)) {
//            if (event.type == sf::Event::Closed) window.close();
//
//            if (event.type == sf::Event::MouseButtonPressed) {
//                sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);
//                for (auto& f : fields) {
//                    if (f.isFocused && !f.box.getGlobalBounds().contains(mousePos)) updateAllCalculations(f);
//                    if (!f.isReadOnly && f.box.getGlobalBounds().contains(mousePos)) {
//                        f.isFocused = true; f.box.setOutlineColor(sf::Color::Yellow);
//                    }
//                    else {
//                        f.isFocused = false; f.box.setOutlineColor(sf::Color(120, 120, 120));
//                    }
//                }
//            }
//
//            if (event.type == sf::Event::TextEntered) {
//                for (auto& f : fields) {
//                    if (!f.isFocused || f.isReadOnly) continue;
//                    if (event.text.unicode == 13 || event.text.unicode == 9) {
//                        updateAllCalculations(f);
//                        f.isFocused = false; f.box.setOutlineColor(sf::Color(120, 120, 120));
//                        // 다음 포커스 이동 로직 
//                        int nextIdx = (f.row * 4 + f.col + 1) % fields.size();
//                        while (fields[nextIdx].isReadOnly) nextIdx = (nextIdx + 1) % fields.size();
//                        fields[nextIdx].isFocused = true; fields[nextIdx].box.setOutlineColor(sf::Color::Yellow);
//                        break;
//                    }
//                    else if (event.text.unicode == 8 && !f.content.empty()) f.content.pop_back();
//                    else if (event.text.unicode < 128 && event.text.unicode >= 32) f.content += static_cast<char>(event.text.unicode);
//                    f.text.setString(f.content);
//                }
//            }
//        }
//
//        window.clear(sf::Color(30, 30, 30));
//        for (auto& f : fields) { window.draw(f.box); window.draw(f.text); }
//
//        // 3. 도형 그리기 (동작 보증)
//        if (displayData.width_mm > 0 && displayData.height_mm > 0) {
//            float scale = 250.0f / std::max(displayData.width_mm, displayData.height_mm);
//            float offsetX = 550.0f, offsetY = 100.0f;
//
//            // Rectangle
//            sf::RectangleShape rect(sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
//            rect.setPosition(offsetX + 150.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
//            rect.setFillColor(sf::Color::Transparent);
//            rect.setOutlineThickness(2);
//            rect.setOutlineColor(sf::Color::White);
//            window.draw(rect);
//
//            // Triangle
//            sf::ConvexShape tri(3);
//            tri.setPoint(0, sf::Vector2f(0, displayData.height_mm * scale));
//            tri.setPoint(1, sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
//            tri.setPoint(2, sf::Vector2f(0, 0));
//            tri.setPosition(offsetX + 450.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
//            tri.setOutlineThickness(2);
//            tri.setOutlineColor(sf::Color::Cyan);
//            window.draw(tri);
//
//            // Labels
//            sf::Text label; label.setFont(font); label.setCharacterSize(12);
//            setupLabel(label, std::to_string((int)displayData.width_mm) + "mm", { rect.getPosition().x + rect.getSize().x / 2, rect.getPosition().y + rect.getSize().y + 15.0f }, 0);
//            window.draw(label);
//            setupLabel(label, std::to_string((int)displayData.height_mm) + "mm", { rect.getPosition().x - 20.0f, rect.getPosition().y + rect.getSize().y / 2 }, -90);
//            window.draw(label);
//            float angle = std::atan2(displayData.height_mm, displayData.width_mm) * 180.0f / PI;
//            setupLabel(label, std::to_string((int)displayData.diagonal_mm) + "mm", { tri.getPosition().x + (displayData.width_mm * scale) / 2 - 10.0f, tri.getPosition().y + (displayData.height_mm * scale) / 2 - 10.0f }, -angle);
//            window.draw(label);
//        }
//        window.display();
//    }
//    return 0;
//}

// 기초 기능 완료 버전 (입력 중 개선 과정에서 버그 ㅜ있음)
//int main() {
//    sf::RenderWindow window(sf::VideoMode(1150, 450), "GeoDisplay Calculator");
//    sf::Font font;
//    if (!font.loadFromFile("c:/windows/fonts/arial.ttf")) {
//        std::cout << "Font load failed!" << std::endl;
//        return -1;
//    }
//
//    std::vector<InputField> fields;
//    DisplayData displayData = { 0, 0, 0 };
//
//    for (int r = 0; r < 6; ++r) {
//        for (int c = 0; c < 4; ++c) {
//            InputField field;
//            field.row = r; field.col = c;
//            if (r == 0) { field.content = colLabels[c]; field.isReadOnly = true; }
//            else if (c == 0) { field.content = rowLabels[r - 1]; field.isReadOnly = true; }
//            else { field.content = ""; field.isReadOnly = false; }
//
//            field.box.setSize({ 110.0f, 30.0f });
//            field.box.setPosition(30.0f + c * 115.0f, 50.0f + r * 35.0f);
//            field.box.setFillColor(field.isReadOnly ? sf::Color(70, 70, 70) : sf::Color(45, 45, 45));
//            field.box.setOutlineThickness(1);
//            field.box.setOutlineColor(sf::Color(120, 120, 120));
//
//            field.text.setFont(font);
//            field.text.setString(field.content);
//            field.text.setCharacterSize(13);
//            field.text.setPosition(field.box.getPosition().x + 5, field.box.getPosition().y + 5);
//            fields.push_back(field);
//        }
//    }
//
//    auto getField = [&](int r, int c) -> InputField& { return fields[r * 4 + c]; };
//
//    while (window.isOpen()) {
//        sf::Event event;
//        while (window.pollEvent(event)) {
//            if (event.type == sf::Event::Closed) window.close();
//
//            if (event.type == sf::Event::MouseButtonPressed) {
//                sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);
//                for (auto& f : fields) {
//                    if (!f.isReadOnly && f.box.getGlobalBounds().contains(mousePos)) {
//                        f.isFocused = true;
//                        f.box.setOutlineColor(sf::Color::Yellow);
//                    }
//                    else {
//                        f.isFocused = false;
//                        f.box.setOutlineColor(sf::Color(120, 120, 120));
//                    }
//                }
//            }
//
//            if (event.type == sf::Event::TextEntered) {
//                for (auto& f : fields) {
//                    if (!f.isFocused || f.isReadOnly) continue;
//
//                    if (event.text.unicode == 8 && !f.content.empty()) f.content.pop_back();
//                    else if (event.text.unicode == 13) f.isFocused = false;
//                    else if (event.text.unicode < 128 && event.text.unicode >= 32) f.content += static_cast<char>(event.text.unicode);
//                    f.text.setString(f.content);
//
//                    try {
//                        // 1 & 2. mm/inch/ratio 직각삼각형 완성 및 상호 변환
//                        auto solveTriangle = [&](int row) {
//                            float w = getField(row, 1).content.empty() ? 0 : std::stof(getField(row, 1).content);
//                            float h = getField(row, 2).content.empty() ? 0 : std::stof(getField(row, 2).content);
//                            float d = getField(row, 3).content.empty() ? 0 : std::stof(getField(row, 3).content);
//
//                            if (w > 0 && h > 0) d = std::sqrt(w * w + h * h);
//                            else if (w > 0 && d > 0) h = std::sqrt(std::max(0.0f, d * d - w * w));
//                            else if (h > 0 && d > 0) w = std::sqrt(std::max(0.0f, d * d - h * h));
//                            else return;
//
//                            getField(row, 1).content = std::to_string(w).substr(0, 7);
//                            getField(row, 2).content = std::to_string(h).substr(0, 7);
//                            getField(row, 3).content = std::to_string(d).substr(0, 7);
//                            };
//
//                        if (f.row == 3 || f.row == 4) { // mm 또는 inch 입력 시
//                            solveTriangle(f.row);
//                            int targetRow = (f.row == 3) ? 4 : 3;
//                            float factor = (f.row == 3) ? (1.0f / 25.4f) : 25.4f;
//                            for (int c = 1; c <= 3; ++c) {
//                                float val = std::stof(getField(f.row, c).content);
//                                getField(targetRow, c).content = std::to_string(val * factor).substr(0, 7);
//                            }
//                            // 3. Ratio 업데이트
//                            float mw = std::stof(getField(3, 1).content);
//                            float mh = std::stof(getField(3, 2).content);
//                            float base = std::min(mw, mh);
//                            getField(2, 1).content = std::to_string(mw / base).substr(0, 6);
//                            getField(2, 2).content = std::to_string(mh / base).substr(0, 6);
//                            getField(2, 3).content = "1.0000";
//                        }
//                        else if (f.row == 2) { // 4. Ratio 수정 시 mm 확장/축소
//                            solveTriangle(2);
//                            float rw = std::stof(getField(2, 1).content);
//                            float rh = std::stof(getField(2, 2).content);
//                            float mw = getField(3, 1).content.empty() ? 0 : std::stof(getField(3, 1).content);
//                            float mh = getField(3, 2).content.empty() ? 0 : std::stof(getField(3, 2).content);
//
//                            if (mw > 0 || mh > 0) {
//                                float currentMax = std::max(mw, mh);
//                                if (rw >= rh) { mw = currentMax; mh = currentMax * (rh / rw); }
//                                else { mh = currentMax; mw = currentMax * (rw / rh); }
//                                getField(3, 1).content = std::to_string(mw);
//                                getField(3, 2).content = std::to_string(mh);
//                                solveTriangle(3); // mm 삼각형 완성
//                                // inch 동기화
//                                for (int c = 1; c <= 3; ++c) {
//                                    getField(4, c).content = std::to_string(std::stof(getField(3, c).content) / 25.4f).substr(0, 7);
//                                }
//                            }
//                        }
//
//                        // 6. Pixel Pitch 계산
//                        float pw = getField(1, 1).content.empty() ? 0 : std::stof(getField(1, 1).content);
//                        float ph = getField(1, 2).content.empty() ? 0 : std::stof(getField(1, 2).content);
//                        float md = getField(3, 3).content.empty() ? 0 : std::stof(getField(3, 3).content);
//                        if (pw > 0 && ph > 0 && md > 0) {
//                            float pitch = md / std::sqrt(pw * pw + ph * ph);
//                            getField(5, 1).content = std::to_string(pitch).substr(0, 8);
//                        }
//
//                        // 5. 렌더링 데이터 업데이트
//                        displayData.width_mm = std::stof(getField(3, 1).content);
//                        displayData.height_mm = std::stof(getField(3, 2).content);
//                        displayData.diagonal_mm = std::stof(getField(3, 3).content);
//
//                        for (auto& field : fields) field.text.setString(field.content);
//                    }
//                    catch (...) {}
//                }
//            }
//        }
//
//        window.clear(sf::Color(30, 30, 30));
//        for (auto& f : fields) { window.draw(f.box); window.draw(f.text); }
//
//        if (displayData.width_mm > 0 && displayData.height_mm > 0) {
//            float scale = 250.0f / std::max(displayData.width_mm, displayData.height_mm);
//            float offsetX = 550.0f, offsetY = 100.0f;
//
//            sf::RectangleShape rect(sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
//            rect.setPosition(offsetX + 150.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
//            rect.setFillColor(sf::Color::Transparent);
//            rect.setOutlineThickness(2);
//            rect.setOutlineColor(sf::Color::White);
//
//            sf::ConvexShape tri(3);
//            tri.setPoint(0, sf::Vector2f(0, displayData.height_mm * scale));
//            tri.setPoint(1, sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
//            tri.setPoint(2, sf::Vector2f(0, 0));
//            tri.setPosition(offsetX + 450.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
//            tri.setOutlineThickness(2);
//            tri.setOutlineColor(sf::Color::Cyan);
//
//            window.draw(rect); window.draw(tri);
//
//            sf::Text label; label.setFont(font); label.setCharacterSize(12);
//            setupLabel(label, std::to_string((int)displayData.width_mm) + "mm", { rect.getPosition().x + rect.getSize().x / 2, rect.getPosition().y + rect.getSize().y + 15.0f }, 0);
//            window.draw(label);
//            setupLabel(label, std::to_string((int)displayData.height_mm) + "mm", { rect.getPosition().x - 20.0f, rect.getPosition().y + rect.getSize().y / 2 }, -90);
//            window.draw(label);
//            float angle = std::atan2(displayData.height_mm, displayData.width_mm) * 180.0f / PI;
//            setupLabel(label, std::to_string((int)displayData.diagonal_mm) + "mm", { tri.getPosition().x + (displayData.width_mm * scale) / 2 - 10.0f, tri.getPosition().y + (displayData.height_mm * scale) / 2 - 10.0f }, -angle);
//            window.draw(label);
//        }
//        window.display();
//    }
//    return 0;
//}

//int main() {
//    // 1. 초기 윈도우 설정 -> 매트릭스 500px + 도형영역 650px
//    sf::RenderWindow window(sf::VideoMode(1150, 450), "GeoDisplay Calculator");
//    sf::Font font;
//    if (!font.loadFromFile("c:/windows/fonts/arial.ttf")) {
//        std::cout << "Font load failed!" << std::endl;
//        return -1;
//    }
//
//    std::vector<InputField> fields;
//    DisplayData displayData = { 0, 0, 0 };
//
//    // 2. 입력 매트릭스 레이아웃 생성 (6행 4열)
//    for (int r = 0; r < 6; ++r) {
//        for (int c = 0; c < 4; ++c) {
//            InputField field;
//            field.row = r; field.col = c;
//
//            // [수정] 특수 공백 제거 및 조건문 정돈
//            if (r == 0) {
//                field.content = colLabels[c];
//                field.isReadOnly = true;
//            }
//            else if (c == 0) {
//                field.content = rowLabels[r - 1];
//                field.isReadOnly = true;
//            }
//            else {
//                field.content = "";
//                field.isReadOnly = false;
//            }
//
//            field.box.setSize({ 110.0f, 30.0f });
//            field.box.setPosition(30.0f + c * 115.0f, 50.0f + r * 35.0f);
//            field.box.setFillColor(field.isReadOnly ? sf::Color(70, 70, 70) : sf::Color(45, 45, 45));
//            field.box.setOutlineThickness(1);
//            field.box.setOutlineColor(sf::Color(120, 120, 120));
//
//            field.text.setFont(font);
//            field.text.setString(field.content);
//            field.text.setCharacterSize(13);
//            field.text.setPosition(field.box.getPosition().x + 5, field.box.getPosition().y + 5);
//
//            fields.push_back(field);
//        }
//    }
//
//    while (window.isOpen()) {
//        sf::Event event;
//        while (window.pollEvent(event)) {
//            if (event.type == sf::Event::Closed) window.close();
//
//            // 3. 마우스 클릭 시 포커스 처리
//            if (event.type == sf::Event::MouseButtonPressed) {
//                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
//                for (auto& f : fields) {
//                    if (!f.isReadOnly && f.box.getGlobalBounds().contains(mousePos)) {
//                        f.isFocused = true;
//                        f.box.setOutlineColor(sf::Color::Yellow);
//                    }
//                    else {
//                        f.isFocused = false;
//                        f.box.setOutlineColor(sf::Color(120, 120, 120));
//                    }
//                }
//            }
//
//            // 4. 키보드 입력 처리
//            if (event.type == sf::Event::TextEntered) {
//                for (auto& f : fields) {
//                    if (f.isFocused && !f.isReadOnly) {
//                        if (event.text.unicode == 8 && !f.content.empty()) f.content.pop_back();
//                        else if (event.text.unicode == 13) f.isFocused = false;
//                        else if (event.text.unicode < 128 && event.text.unicode >= 32) f.content += static_cast<char>(event.text.unicode);
//
//                        f.text.setString(f.content);
//
//                        try {
//                            // [오작동 1 해결] mm <-> inch 실시간 변환 로직 우선 실행
//                            if (!f.content.empty() && f.content != ".") {
//                                float val = std::stof(f.content);
//                                if (f.row == 3) { // mm 수정 시 inch 행 업데이트
//                                    for (auto& target : fields) {
//                                        if (target.row == 4 && target.col == f.col) {
//                                            target.content = std::to_string(val / 25.4f).substr(0, 6);
//                                            target.text.setString(target.content);
//                                        }
//                                    }
//                                }
//                                else if (f.row == 4) { // inch 수정 시 mm 행 업데이트
//                                    for (auto& target : fields) {
//                                        if (target.row == 3 && target.col == f.col) {
//                                            target.content = std::to_string(val * 25.4f).substr(0, 6);
//                                            target.text.setString(target.content);
//                                        }
//                                    }
//                                }
//                            }
//
//                            // [오작동 2&3 해결] 렌더링 및 계산용 데이터 전체 스캔
//                            float pxW = 0, pxH = 0, current_mmD = 0;
//                            for (auto& field : fields) {
//                                if (field.content.empty() || field.content == ".") continue;
//                                float val = std::stof(field.content);
//
//                                if (field.row == 1) { // Pixels
//                                    if (field.col == 1) pxW = val;
//                                    if (field.col == 2) pxH = val;
//                                }
//                                if (field.row == 3) { // Millimeters
//                                    if (field.col == 1) displayData.width_mm = val;
//                                    if (field.col == 2) displayData.height_mm = val;
//                                    if (field.col == 3) { current_mmD = val; displayData.diagonal_mm = val; }
//                                }
//                            }
//
//                            // Pixel Pitch 계산 (6행 2열)
//                            if (pxW > 0 && pxH > 0 && current_mmD > 0) {
//                                float pixelPitch = current_mmD / std::sqrt(pxW * pxW + pxH * pxH);
//                                for (auto& target : fields) {
//                                    if (target.row == 5 && target.col == 1) {
//                                        target.content = std::to_string(pixelPitch).substr(0, 8);
//                                        target.text.setString(target.content);
//                                    }
//                                }
//                            }
//                        }
//                        catch (...) {}
//                    }
//                }
//            }
//        }
//
//        window.clear(sf::Color(30, 30, 30));
//
//        // UI 매트릭스 렌더링
//        for (auto& f : fields) {
//            window.draw(f.box);
//            window.draw(f.text);
//        }
//
//        // [수정] 실시간 그래픽 렌더링 영역 (displayData 기반)
//        if (displayData.width_mm > 0 && displayData.height_mm > 0) {
//            float scale = 250.0f / std::max(displayData.width_mm, displayData.height_mm);
//            float offsetX = 550.0f;
//            float offsetY = 100.0f;
//
//            sf::RectangleShape rect(sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
//            rect.setPosition(offsetX + 150.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
//            rect.setFillColor(sf::Color::Transparent);
//            rect.setOutlineThickness(2);
//            rect.setOutlineColor(sf::Color::White);
//
//            sf::ConvexShape tri(3);
//            tri.setPoint(0, sf::Vector2f(0, displayData.height_mm * scale));
//            tri.setPoint(1, sf::Vector2f(displayData.width_mm * scale, displayData.height_mm * scale));
//            tri.setPoint(2, sf::Vector2f(0, 0));
//            tri.setPosition(offsetX + 450.0f - (rect.getSize().x / 2.0f), offsetY + 150.0f - (rect.getSize().y / 2.0f));
//            tri.setOutlineThickness(2);
//            tri.setOutlineColor(sf::Color::Cyan);
//
//            window.draw(rect);
//            window.draw(tri);
//
//            sf::Text label; label.setFont(font); label.setCharacterSize(12);
//            setupLabel(label, std::to_string((int)displayData.width_mm) + "mm",
//                { rect.getPosition().x + rect.getSize().x / 2, rect.getPosition().y + rect.getSize().y + 15.0f }, 0);
//            window.draw(label);
//
//            setupLabel(label, std::to_string((int)displayData.height_mm) + "mm",
//                { rect.getPosition().x - 15.0f, rect.getPosition().y + rect.getSize().y / 2 }, -90);
//            window.draw(label);
//
//            float angle = std::atan2(displayData.height_mm, displayData.width_mm) * 180.0f / 3.141592f;
//            setupLabel(label, std::to_string((int)displayData.diagonal_mm) + "mm",
//                { tri.getPosition().x + (displayData.width_mm * scale) / 2 - 10.0f, tri.getPosition().y + (displayData.height_mm * scale) / 2 - 10.0f }, -angle);
//            window.draw(label);
//        }
//        window.display();
//    }
//    return 0;
//}