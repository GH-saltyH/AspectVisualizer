GeoDisplay Calculator
GeoDisplay Calculator는 디스플레이의 물리적 규격과 해상도 데이터를 기반으로 화면 비율(Ratio), mm/inch 단위 길이, 픽셀 피치(Pixel Pitch)를 정밀하게 계산하고 시각화하는 가벼운 C++/SFML 기반 엔지니어링 도구입니다.

■ 주요 기능 (Key Features)
● 실시간 직각삼각형 완성 (Triangulation)

가로, 세로, 대각선 중 두 가지 이상의 값이 입력되면 피타고라스 정리를 활용하여 나머지 한 변을 자동으로 산출합니다.

사용자가 대각선(Diagonal)을 입력할 경우, 기존의 가로/세로 비율 중 큰 값을 기준으로 형태를 유지하며 크기를 자동 조정합니다.

● 유기적 단위 동기화 (Unit Synchronization)

밀리미터(mm)와 인치(in) 사이의 값을 실시간으로 변환하며, 한 쪽의 수정사항이 즉시 반대편 단위에 반영됩니다.

mm 또는 inch 값의 변화에 따라 화면 비율(Ratio)이 소수점 4자리 정밀도로 자동 갱신됩니다.

● 디스플레이 정밀도 분석 (PPI Analysis)

입력된 픽셀 해상도(Pixels)와 물리적 대각선 길이를 결합하여 픽셀 피치(Pixel Pitch)를 계산합니다.

● 기하학적 실시간 시각화 (Geometry Visualization)

계산된 수치를 바탕으로 사각형(화면 영역)과 직각삼각형(구조)을 실시간 렌더링합니다.

빗변의 기울기에 맞춰 정렬된 수치 레이블을 통해 직관적인 데이터 확인이 가능합니다.

● 사용자 중심의 입력 인터페이스

Enter 또는 Tab 키를 이용한 스마트 포커스 이동 시스템을 제공합니다.

결과값 전용 필드(Pixel Pitch, Ratio Diagonal)에 대한 쓰기 방지(Read-only)를 통해 데이터 무결성을 보장합니다.

■ 기술 스택 및 라이브러리 (Tech Stack)
● Core Language: C++ (C++17 이상 권장)
● Graphics Library: SFML (Simple and Fast Multimedia Library)
● OS 환경: Windows (폰트 경로: C:/Windows/Fonts/Arial.ttf 참조)

■ 사용 방법 (Usage Instructions)
Pixels(px): 디스플레이의 가로 및 세로 해상도를 입력합니다.

Ratio/Constant: 원하는 화면 비율을 설정합니다. mm 값이 존재할 경우 해당 비율로 크기가 확장/축소됩니다.

Millimeters / Inches: 물리적 크기를 입력합니다. 입력 완료(Enter/Tab/Focus Lost) 시 모든 연관 데이터가 업데이트됩니다.

Summary: 설정 공유를 위해 각 섹션마다 주요 지점을 요약하는 기능을 활용할 수 있습니다.