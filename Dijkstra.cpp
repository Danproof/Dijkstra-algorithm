#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <windows.h>
#include "stb_image.h"
#include<algorithm>
using namespace std;

#define Q 51 
#define q 113
#define W 87
#define w1 119
#define E 69
#define e 101
#define R 82
#define r 114
#define T 84
#define t 116
#define INFINITY 100000
float w = 400, h = 400;
float mx, my; // координати положення курсора
int button = -1; // число що відповідає за кнопки
bool button0 = 0; // 1 - перетаскуємо вершину
bool button2 = 0; // 0 - натисніть на першу вершину, 1 - на другу
bool button3 = 0; // 0 - оберыть вершину, з якої шукаємо найкоротші відстані
bool connect = 0; // 1 - меню побудови ребра
int iter = 0; // комірка масиву з ребром яке буде намальоване
int n = 0;

struct point {
    float x;
    float y;
    bool operator ==(const point& another) const {
        return (x == another.x && y == another.y);
    }
};

struct edge {
    point v2;
    float weight = 0;
    bool opposite = 0;
    bool exists = 0;
    edge(point v) {
        v2 = v;
    }
    bool operator <(const edge& another) const {
        return (weight < another.weight);
    }
};

struct vertex {
    point v;
    vector<edge> edges;
    bool chosen = 0;
    int path = INFINITY;
    bool marked = 0;
    vertex() {};
    vertex(float x, float y) {
        v.x = x;
        v.y = y;
    }
    bool operator !=(const vertex& another) const {
        return (v.x != another.v.x || v.y != another.v.y);
    }
};

vector<vertex> vertices;
pair<vertex*, vertex*> CV;
vertex* from;
vertex* motion;

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_POINT_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
}

void reshape(int new_w, int new_h)
{
    w = new_w;
    h = new_h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
}

void text(float x, float y, string s, void* font) {
    glRasterPos2f(x, y);
    glutBitmapString(font, (const unsigned char*)s.c_str());
}

template <typename type>
string tostring(type value) {
    stringstream S;
    S << value;
    return S.str();
}

void menu() {
    float bw = w / 5;
    glColor3f(0, 0, 0);
    glRectf(0, h - 50, w, h);
    glColor3f(1, 1, 1);
    glRectf(3, h - 47, w - 3, h - 3);
    glColor3f(0.6, 0.6, 1);
    for (int i = 0; i < 5; i++) {
        if (i == button) {
            glRectf(i * bw, h - 47, (i + 1) * bw, h - 3);
            break;
        }
    }
    glColor3f(0, 0, 0);
    glLineWidth(3);
    glBegin(GL_LINES);
    for (int i = 1; i < 5; i++) {
        glVertex2f(i * bw, h - 50);
        glVertex2f(i * bw, h);
    }
    glEnd();
    text(bw / 2.5, h - 30, "Default", GLUT_BITMAP_HELVETICA_18);
    text(bw + bw / 3, h - 30, "Add vertex", GLUT_BITMAP_HELVETICA_18);
    text(2 * bw + bw / 4 - 20, h - 30, "Connect / Disconnect", GLUT_BITMAP_HELVETICA_18);
    text(3 * bw + bw / 4, h - 30, "Find shortest path", GLUT_BITMAP_HELVETICA_18);
    text(4 * bw + bw / 4, h - 30, "Remove vertex", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.5, 0.5, 0.5);
    text(5, h - 20, "Q", GLUT_BITMAP_HELVETICA_18);
    text(bw + 5, h - 20, "W", GLUT_BITMAP_HELVETICA_18);
    text(2 * bw + 5, h - 20, "E", GLUT_BITMAP_HELVETICA_18);
    text(3 * bw + 5, h - 20, "R", GLUT_BITMAP_HELVETICA_18);
    text(4 * bw + 5, h - 20, "T", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0, 0, 1);
    switch (button) {
    case 0: text(5, h - 70, "Select and move objects by mouse", GLUT_BITMAP_9_BY_15); break;
    case 1: text(5, h - 70, "Click to workspace to add a new vertex", GLUT_BITMAP_9_BY_15); break;
    case 2:
        if (button2 == 0)
            text(5, h - 70, "Select first vertex of edge", GLUT_BITMAP_9_BY_15);
        else
            text(5, h - 70, "Select second vertex of edge", GLUT_BITMAP_9_BY_15);
        break;
    case 3:text(5, h - 70, "Select the initial vertex of the shortest path", GLUT_BITMAP_9_BY_15); break;
    case 4: text(5, h - 70, "Click on the object to remove", GLUT_BITMAP_9_BY_15); break;
    }
    if (connect == 1) {
        glColor3f(0, 0, 0);
        glRectf(500, 300, 800, 500);
        glColor3f(1, 1, 1);
        glRectf(503, 303, 797, 497);
        glColor3f(1, 0, 0);
        glRectf(775, 475, 795, 495);
        glColor3f(0, 0, 0);
        glRectf(680, 400, 795, 430);
        glRectf(720, 370, 740, 390);
        glRectf(745, 370, 765, 390);
        glColor3f(1, 1, 1);
        glRectf(777, 477, 793, 493);
        glRectf(682, 402, 793, 428);
        glRectf(722, 372, 738, 388);
        glRectf(747, 372, 763, 388);
        glBegin(GL_LINES);
        glColor3f(0, 0, 0);
        glVertex2f(500, 340);
        glVertex2f(800, 340);
        glVertex2f(650, 300);
        glVertex2f(650, 340);
        glEnd();
        glColor3f(0, 0, 0);
        text(510, 475, "Add or delete edge", GLUT_BITMAP_HELVETICA_18);
        text(780, 480, "x", GLUT_BITMAP_HELVETICA_18);
        text(510, 410, "Edge weight", GLUT_BITMAP_HELVETICA_18);
        text(725, 375, "+", GLUT_BITMAP_HELVETICA_18);
        text(750, 375, "-", GLUT_BITMAP_HELVETICA_18);
        text(685, 405, tostring(CV.first->edges[iter].weight), GLUT_BITMAP_HELVETICA_18);
        text(555, 315, "Add", GLUT_BITMAP_HELVETICA_18);
        text(700, 315, "Delete", GLUT_BITMAP_HELVETICA_18);
        glColor3f(0, 0, 0);
    }
}

unsigned int texture, texture2, texture3;
void circletexture() {
    int width, height, cnt;
    unsigned char* data = stbi_load("f.png", &width, &height, &cnt, 0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned char* data2 = stbi_load("s.png", &width, &height, &cnt, 0);
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned char* data3 = stbi_load("i.png", &width, &height, &cnt, 0);
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data3);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void circle(const point& v, bool ch) {
    glEnable(GL_TEXTURE_2D);
    if (ch == 0)
        glBindTexture(GL_TEXTURE_2D, texture);
    else
        glBindTexture(GL_TEXTURE_2D, texture2);
    float x1 = v.x - 18, y1 = v.y - 18, x2 = v.x + 18, y2 = v.y + 18;
    float arr[] = { x1,y1, x2,y1, x2,y2, x1,y2 };
    float texCoord[] = { 0,1, 1,1, 1,0, 0,0 };
    glColor3f(1, 1, 1);
    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, arr);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
    glColor3f(0, 0, 0);
    if (n < 10) {
        text(v.x - 5, v.y - 5, tostring(n), GLUT_BITMAP_HELVETICA_18);
    }
    else {
        if (n < 100)
            text(v.x - 10, v.y - 5, tostring(n), GLUT_BITMAP_HELVETICA_18);
        else
            text(v.x - 10, v.y - 3, tostring(n), GLUT_BITMAP_HELVETICA_12);
    }
}
void add_edge(const vertex& vertex, const edge& edge) { 
    float cx1, cx2, cy1, cy2;//рівняння кола (х-сх1)^2 + (y-cy1)^2 = 18^2 = 324
    cx1 = vertex.v.x;
    cy1 = vertex.v.y;
    cx2 = edge.v2.x;
    cy2 = edge.v2.y;
    float a, b; // рівняння прямої y = ax + b
    double D; // дискримінант
    double Da, Db, Dc; // константи дискримінанту
    float x1, x2; // точка претину на колі
    float X1, X2, Y1, Y2; // координати кінців ребра
    float t1x, t2x, t1y, t2y;
    float x_, k, K, delta;
    float tx, ty;
        
    a = (cy2 - cy1) / (cx2 - cx1);
    b = cy1 - a * cx1;
        
    Da = (1 + a * a); // знаходимо точки  перетину на першому колі
    Db = 2 * (a * (b - cy1) - cx1);
    Dc = (cx1 * cx1 + (b - cy1) * (b - cy1) - 300);
    D = Db * Db - 4 * Da * Dc;
    x1 = (-Db - sqrt(D)) / (2 * Da);
    x2 = (-Db + sqrt(D)) / (2 * Da);
    if (cx1 < cx2)
        X1 = x2;
    else
        X1 = x1;
    Db = 2 * (a * (b - cy2) - cx2);// знаходимо точки  перетину на другому колі
    Dc = (cx2 * cx2 + (b - cy2) * (b - cy2) - 300);
    D = Db * Db - 4 * Da * Dc;
    x1 = (-Db - sqrt(D)) / (2 * Da);
    x2 = (-Db + sqrt(D)) / (2 * Da);
    if (cx1 < cx2)
        X2 = x1;
    else
        X2 = x2;
    Y1 = a * X1 + b;
    Y2 = a * X2 + b;
    glColor3f(0, 0, 1);
    float cx, cy; // координати центра ребра
    cx = (cx1 + cx2) / 2;
    cy = (cy1 + cy2) / 2;
    if (edge.opposite == 0) {
        glBegin(GL_LINES);
        glVertex2f(X1, Y1);
        glVertex2f(X2, Y2);
        glEnd();
    }
    else {// знайдемо точку віддалену від центру ребра, що буде лежати на зміщеній дотичній до вершини, і проведемо через неї ребро
        float d = sqrt((X1 - X2) * (X1 - X2) + (Y1 - Y2) * (Y1 - Y2)) / 15; // відстань від шуканої точки до центру відрізка
        k = -(X2 - cx2) / (Y2 - cy2);
        x_ = (cy - Y2) / k + X2;
        delta = x_ - cx;
        K = -k * X2 + k * delta - cy + Y2;
        Da = k * k + 1;
        Db = 2 * (k * K - cx);
        Dc = K * K + cx * cx - d * d;
        D = Db * Db - 4 * Da * Dc;
        if (cx1 < cx2) 
            tx = (-Db - sqrt(D)) / (2 * Da);
        else
            tx = (-Db + sqrt(D)) / (2 * Da);
        ty = Y2 + k * (tx - X2 + delta); // шукана точка
        a = (ty - cy1) / (tx - cx1); // знаходимо точку  перетину на першому колі
        b = cy1 - a * cx1;
        Da = (1 + a * a);
        Db = 2 * (a * (b - cy1) - cx1);
        Dc = (cx1 * cx1 + (b - cy1) * (b - cy1) - 324);
        D = Db * Db - 4 * Da * Dc;
        x1 = (-Db - sqrt(D)) / (2 * Da);
        x2 = (-Db + sqrt(D)) / (2 * Da);
        if (cx1 < cx2) {
            if (cx1 < tx)
                X1 = x2;
            else
                X1 = x1;
        }
        else {
            if (cx1 < tx)
                X1 = x2;
            else
                X1 = x1;
        }
        Y1 = a * X1 + b;
        a = (ty - cy2) / (tx - cx2); // знаходимо точку  перетину на другому колі
        b = cy2 - a * cx2;
        Da = (1 + a * a);
        Db = 2 * (a * (b - cy2) - cx2);
        Dc = (cx2 * cx2 + (b - cy2) * (b - cy2) - 324);
        D = Db * Db - 4 * Da * Dc;
        x1 = (-Db - sqrt(D)) / (2 * Da);
        x2 = (-Db + sqrt(D)) / (2 * Da);
        if (cx1 < cx2)
                X2 = x1;
            else
                X2 = x2;
        Y2 = a * X2 + b;
        glBegin(GL_LINE_STRIP);
        glVertex2f(X1, Y1);
        glVertex2f(tx, ty);
        glVertex2f(X2, Y2);
        glEnd();
    }
    if (edge.opposite) {
        cx = tx; 
        cy = ty;
    }
    string s = tostring(edge.weight);
    int l = s.length();
    void* font;
    float y1, y2;
    if (l > 4) {
        l *= 4;
        l += 2;
        font = GLUT_BITMAP_HELVETICA_12;
        y1 = cy - 8;
        y2 = cy + 8;
    }
    else {
        if (l < 2)
            l *= 8;
        else
            l *= 7;
        font = GLUT_BITMAP_HELVETICA_18;
        y1 = cy - 10;
        y2 = cy + 10;
    }
    x1 = cx - l;
    x2 = cx + l;
    glRectf(x1, y1, x2, y2);
    glColor3f(1, 1, 1);
    glRectf(x1 + 2, y1 + 2, x2 - 2, y2 - 2);
    glColor3f(0, 0, 0);
    text(x1 + 4, y1 + 4, s, font);
    Dc = (cx2 * cx2 + (b - cy2) * (b - cy2) - 1000);
    D = Db * Db - 4 * Da * Dc;// дискримінант для tx коли радіус другої вершини sqrt(1000)
    if (cx1 > cx2) {
        tx = (-Db + sqrt(D)) / (2 * Da);
    }
    else {
        tx = (-Db - sqrt(D)) / (2 * Da);
    }
    ty = a * tx + b; // точка в яку опускається висота рівнобедреного трикутника що утворює стрілку
    if (abs(Y2 - cy2) > 1) {
        k = -(X2 - cx2) / (Y2 - cy2); // коефіцієнт дотичної перед х у рівнянні дотичної до кола
        x_ = (ty - Y2) / k + X2; // координата х, що залежить від ty, ця точка лежить на дотичній до кола
        delta = x_ - tx; // зміщення дотичної
        K = -k * X2 + k * delta - ty + Y2; // константа в квадратному рівнянні, яке отримуємо при розв'язку системи
        Da = k * k + 1;
        Db = 2 * (k * K - tx);
        Dc = K * K + tx * tx - 70;
        D = Db * Db - 4 * Da * Dc;
        t1x = (-Db - sqrt(D)) / (2 * Da); // координати точок, що лежать на зміщеній дотичній на відстані 10 від точки з координатами tx, ty
        t2x = (-Db + sqrt(D)) / (2 * Da);
        t1y = Y2 + k * (t1x - X2 + delta);
        t2y = Y2 + k * (t2x - X2 + delta);
        glColor3f(0, 0, 1);
        glBegin(GL_TRIANGLES);
        if (cx2 > X2)
            glVertex2f(X2 + 2, Y2);
        else
            glVertex2f(X2 - 2, Y2);
        glVertex2f(t1x, t1y);
        glVertex2f(t2x, t2y);
        glEnd();
    }
    else {
        glColor3f(0, 0, 1);
        glBegin(GL_TRIANGLES);
        if (cx2 > X2)
            glVertex2f(X2 + 2, Y2);
        else
            glVertex2f(X2 - 2, Y2);
        glVertex2f(tx, ty + 8);
        glVertex2f(tx, ty - 8);
        glEnd();
    }
}
void objects() {
    for (const auto& vert : vertices) {
        circle(vert.v, vert.chosen); // малюємо вершини
        n++;
    }
    n = 0;
    for (const auto& vert : vertices) {
        for (const auto& ed : vert.edges) {
            if (ed.exists == 1) {
                add_edge(vert, ed); // малюємо ребра
            }
        }
        n++;
    }
    n = 0;
    if (button3) {
        for (auto&vert : vertices) {
            if (vert != *from) {
                if (vert.path == INFINITY) {
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, texture3);
                    float x1 = vert.v.x - 12, y1 = vert.v.y + 18, x2 = vert.v.x + 12, y2 = y1 + 14;
                    float arr[] = { x1,y1, x2,y1, x2,y2, x1,y2 };
                    float texCoord[] = { 0,1, 1,1, 1,0, 0,0 };
                    glColor3f(1, 1, 1);
                    glPushMatrix();
                    glEnableClientState(GL_VERTEX_ARRAY);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glVertexPointer(2, GL_FLOAT, 0, arr);
                    glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    glPopMatrix();
                }
                else {
                    string s = tostring(vert.path);
                    int l = s.length();
                    void* font;
                    float x1, x2, y1, y2;
                    y1 = vert.v.y + 18;
                    if (l > 4) {
                        l *= 4;
                        l += 2;
                        font = GLUT_BITMAP_HELVETICA_12;
                        y2 = y1 + 14;
                    }
                    else {
                        if (l < 2)
                            l *= 8;
                        else
                            l *= 7;
                        font = GLUT_BITMAP_HELVETICA_18;
                        y2 = y1 + 20;
                    }
                    x1 = vert.v.x - l;
                    x2 = vert.v.x + l;
                    glColor3f(1, 1, 1);
                    glRectf(x1, y1, x2, y2);
                    glColor3f(1, 1, 1);
                    glRectf(x1 + 2, y1 + 2, x2 - 2, y2 - 2);
                    glColor3f(0, 0, 0);
                    text(x1 + 4, y1 + 4, s, font);
                }
            }
        }
    }
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    objects();
    menu();
    glutSwapBuffers();
}

void mousebutton(int button_, int state, int x, int y) {
    if (button_ == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        mx = x;
        my = h - y;
        if (connect == 1) { //додавання ребра
            if (mx > 775 && my > 475 && mx < 795 && my < 495) { // вихід
                connect = 0;
                CV.first->chosen = 0;
                CV.second->chosen = 0;
                iter = 0;
            }
            if (mx > 722 && my > 372 && mx < 738 && my < 388) { //+
                CV.first->edges[iter].weight++;
            }
            if (mx > 747 && my > 372 && mx < 763 && my < 388) { //-
                if (CV.first->edges[iter].weight > 0)
                    CV.first->edges[iter].weight--;
            }
            if (mx > 500 && my > 300 && mx < 650 && my < 340) { // додати
                connect = 0;
                CV.first->chosen = 0;
                CV.second->chosen = 0;
                CV.first->edges[iter].exists = 1;
                for (auto& element : CV.second->edges) {
                    if (element.v2 == CV.first->v) {
                        CV.first->edges[iter].opposite = element.opposite = 1;
                        break;
                    }
                }
                iter = 0;
            }
            if (mx > 650 && my > 300 && mx < 800 && my < 340) { // видалити
                connect = 0;
                CV.first->chosen = 0;
                CV.second->chosen = 0;
                if (CV.first->edges[iter].opposite) {
                    for (auto& ed : CV.second->edges) {
                        if (ed.v2 == CV.first->v) {
                            ed.opposite = 0;
                            break;
                        }
                    }
                }
                CV.first->edges.erase(CV.first->edges.begin() + iter);
                iter = 0;
            }
        }
        else {
            float bw = w / 5;
            for (int i = 0; i < 5; i++) {
                if (mx > i * bw && mx < (i + 1) * bw && my > h - 50 && my < h) {
                    button = i;
                    break;
                }
            }
            if (button == 0) {
                button0 = 0;
            }
            if (button != 2 && button2 == 1) {
                CV.first->chosen = 0;
                button2 = 0;
            }
            if (button != 3 && button3 == 1) {
                button3 = 0;
                from->chosen = 0;
                for (auto& vert : vertices) {
                    vert.marked = 0;
                    vert.path = INFINITY;
                }
            }
            if (my < h - 50) {
                switch (button) {
                case 1:
                    for (const auto& element : vertices) {
                        if (fabs(mx - element.v.x) < 36 && fabs(my - element.v.y) < 36)
                            return;
                    }
                    if (vertices.size() < 999)
                        vertices.push_back(vertex(mx, my));
                    break;
                case 2:
                    int i;
                    i = 0;
                    for (auto& element : vertices) {
                        if (fabs(mx - element.v.x) < 18 && fabs(my - element.v.y) < 18) {
                            if (button2 == 0) {
                                CV.first = &element;
                                CV.first->chosen = 1;
                                button2 = 1;
                            }
                            else {
                                if (CV.first != &element) {
                                    CV.second = &element;
                                    CV.second->chosen = 1;
                                    button2 = 0;
                                    for (auto& element2 : CV.first->edges) {
                                        if (element2.v2 == element.v) {
                                            break;
                                        }
                                        iter++;
                                    }
                                    if (iter == CV.first->edges.size())
                                        CV.first->edges.push_back(edge(element.v));
                                    connect = 1;    
                                }
                                else {
                                    CV.first->chosen = 0;
                                    button2 = 0;
                                }
                            }
                            break;
                        }
                        i++;
                    }
                    if (i == vertices.size() && button2 == 1) {
                        CV.first->chosen = 0;
                        button2 = 0;
                    }
                    break;
                case 3:
                    for (auto& vert : vertices) {
                        if (fabs(mx - vert.v.x) < 18 && fabs(my - vert.v.y) < 18) {
                            if (button3 == 1) {
                                if (*from != vert) {
                                    for (auto& vert : vertices) {
                                        vert.marked = 0;
                                        vert.path = INFINITY;
                                    } 
                                    from->chosen = 0;
                                }
                                else 
                                    break;
                            }
                            button3 = 1;
                            from = &vert;
                            vert.chosen = 1;
                            vert.path = 0;
                            int path = INFINITY;
                            vertex* current = &vert;
                            for (int i = 0; i < vertices.size(); i++) {
                                for (auto& vert : vertices) {
                                    if (vert.marked == 0)
                                        if (path > vert.path) {
                                            current = &vert;
                                            path = current->path;
                                        }
                                }
                                for (const auto& ed : current->edges) {
                                    for (auto& vert : vertices) {
                                        if (vert.v.x == ed.v2.x && vert.v.y == ed.v2.y) {
                                            if (vert.marked == 0)
                                                if (vert.path > path + ed.weight)
                                                    vert.path = path + ed.weight;
                                        }
                                    }
                                }
                                current->marked = 1;
                                path = INFINITY;
                            }
                            break;
                        }
                    }
                    break;
                case 4:
                    for (auto i = vertices.begin(); i != vertices.end();) {
                        if (fabs(mx - i->v.x) < 18 && fabs(my - i->v.y) < 18)
                            i = vertices.erase(i);  
                        else {
                            for (auto j = i->edges.begin(); j != i->edges.end();){
                                if (fabs(mx - j->v2.x) < 18 && fabs(my - j->v2.y) < 18)
                                    j = i->edges.erase(j);
                                else
                                j++;
                            }
                            i++;
                        }
                    }
                    break;
                }
            }
        }
    }
}

void mousemove(int x, int y) {
    if (button == 0) {
        y = h - y;
        if (!(x<18 || x>w - 18 || y<18 || y>h - 68)) {
            if (button0 == 0) {
                for (auto& vert : vertices) {
                    if (fabs(x - vert.v.x) < 18 && fabs(y - vert.v.y) < 18) {
                        motion = &vert;
                        button0 = 1;
                        break;
                    }
                }
            }
            if (button0 == 0)
                return;
            for (auto& vert : vertices) {
                if (vert != *motion && fabs(x - vert.v.x) < 36 && fabs(y - vert.v.y) < 36)
                    return;
            }
            for (auto& vert : vertices) {
                for (auto& ed : vert.edges) {
                    if (ed.v2 == motion->v) {
                        ed.v2.x = x;
                        ed.v2.y = y;
                    }
                }
            }
            motion->v.x = x;
            motion->v.y = y;
            glutPostRedisplay();
        }
    }
}

void keyboard(unsigned char key, int x, int y)
{
    if (connect == 0) {
        switch (key)
        {
        case Q: // вимкнені дії
        case q:
            button = 0;
            glutPostRedisplay();
            break;
        case W: // додати вершину
        case w1:
            button = 1;
            glutPostRedisplay();
            break;
        case E: // з'єднати вершини
        case e:
            button = 2;

            glutPostRedisplay();
            break;
        case R: // Дейкстрі
        case r:
            button = 3;

            glutPostRedisplay();
            break;
        case T: // видалити елемент
        case t:
            button = 4;
            glutPostRedisplay();
            break;
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(400, 150);
    glutCreateWindow("WebWindow");
    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    init();
    glutMotionFunc(mousemove);
    glutMouseFunc(mousebutton);
    glutKeyboardFunc(keyboard);
    circletexture();
    glutMainLoop();
}
