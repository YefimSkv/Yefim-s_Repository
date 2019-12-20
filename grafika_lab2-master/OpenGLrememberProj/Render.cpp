#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <array>
#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"
std::array<double, 3> VectorN(double* N1, double* N2, double* N3);
bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//попробовать сделать крышку прозрачной (последнее)
	
	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	////double A[2] = { -4, -4 };
	////double B[2] = { 4, -4 };
	////double C[2] = { 4, 4 };
	////double D[2] = { -4, 4 };
	//
	//glBindTexture(GL_TEXTURE_2D, texId);
	//
	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);
	//
	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);
	//
	//glEnd();
	double A[] = { 0,0,0 };
	double B[] = { -6,4,0 };
	double C[] = { -10,-1,0 };
	double D[] = { 1,-6,0 };
	double E[] = { 6,-2,0 };
	double F[] = { 3,6,0 };
	double G[] = { -1,6,0 };
	double n1 = 0; 
	double n2 = 0;
	double	n3 = 0;
	std::array<double, 3>N;

	glNormal3d(0, 0, -1);

	glBegin(GL_TRIANGLES);
	//N = VectorN(A, B, C);
	//glNormal3d(N[0], N[1], N[2]);
	glColor3d(0.5, 0.3, 0.6);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);

	glColor3d(0.5, 0.3, 0.6);
	//N = VectorN(A, C, D);
	//glNormal3d(N[0], N[1], N[2]);
	glVertex3dv(A);
	glVertex3dv(C);
	glVertex3dv(D);

	glColor3d(0.5, 0.3, 0.6);
	//N = VectorN(A, E, D);
	//glNormal3d(N[0], N[1], N[2]);
	glVertex3dv(A);
	glVertex3dv(E);
	glVertex3dv(D);
	glEnd();



	glBegin(GL_QUADS);
	glColor3d(0.5, 0.3, 0.6);
	//N = VectorN(A, E, G);
	//glNormal3d(N[0], N[1], N[2]);
	glVertex3dv(A);
	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(G);
	glEnd();

	//верхнее основание
	double A1[] = { 0,0,4 };
	double B1[] = { -6,4,4 };
	double C1[] = { -10,-1,4 };
	double D1[] = { 1,-6,4 };
	double E1[] = { 6,-2,4 };
	double F1[] = { 3,6,4 };
	double G1[] = { -1,6,4 };
	glNormal3d(0, 0, 1);

	glBegin(GL_TRIANGLES);
	glColor3d(0.5, 0.5, 0.3);
	//N = VectorN(A1, B1, C1);
	//glNormal3d(N[0], N[1], N[2]);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);

	glColor3d(0.5, 0.5, 0.3);
	//N = VectorN(A1, C1, D1);
	//glNormal3d(N[0], N[1], N[2]);
	glVertex3dv(A1);
	glVertex3dv(C1);
	glVertex3dv(D1);

	glColor3d(0.5, 0.5, 0.3);
	//N = VectorN(A1, E1, D1);
	//glNormal3d(N[0], N[1], N[2]);
	glVertex3dv(A1);
	glVertex3dv(E1);
	glVertex3dv(D1);



	glEnd();
	glBegin(GL_QUADS);

	//N = VectorN(A1, E1, F1);
	//glNormal3d(N[0], N[1], N[2]);
	glColor3d(0.5, 0.5, 0.3);
	glVertex3dv(A1);
	glVertex3dv(E1);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glEnd();// конец верхнего основания

	//грани боковой поверхности
	glBegin(GL_QUADS);
	
	N = VectorN(A1, A, B1);
	glNormal3d(N[0], N[1], N[2]);
	glColor3d(0.9, 0.2, 0.5);
	glVertex3dv(A1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(B1);

	
	N = VectorN(B1, B, C1);//?
	glNormal3d(N[0], N[1], N[2]);
	glColor3d(0.9, 0.2, 0.5);	
	glVertex3dv(B1);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(C1);


	N = VectorN(C1, C, D1);
	glNormal3d(N[0],  N[1], N[2]);
	glColor3d(0.9, 0.2, 0.5);
	glVertex3dv(C1);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(D1);


	
	//N = VectorN(E1, E, D1);//?	
	//glNormal3d(N[0], (-1)*N[1], N[2]);//glNormal3d( 16, -20, 0);// можно заменить на это, это просто уже посчитано и записано в цифрах
	//glNormal3d(16, -20, 0);
	N = VectorN(E1, E, D1);//?	
	glNormal3d((-1)*N[0], (-1)*N[1], N[2]);
	glColor3d(0.9, 0.2, 0.5);
	glVertex3dv(D1);
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(E1);


	N = VectorN(F, F1, E);
	glNormal3d(N[0], N[1], N[2]);
	glColor3d(0.7, 0.2, 0.2);
	glVertex3dv(F1);
	glVertex3dv(F);
	glVertex3dv(E);
	glVertex3dv(E1);


	N = VectorN(F1, F, G1);
	glNormal3d(N[0],  N[1], N[2]);
	glColor3d(0.9, 0.2, 0.5);
	glVertex3dv(F1);
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(G1);


	
	N = VectorN(G1, G, A1);
	glNormal3d(N[0], N[1],N[2]);
	glColor3d(0.9, 0.2, 0.5);
	glVertex3dv(G1);
	glVertex3dv(G);
	glVertex3dv(A);
	glVertex3dv(A1);


	glEnd();

	//полуцилиндр
	double O[] = { 4.5, 2,0 };
	double O2[] = { 4.5, 2,4 };
	double x1 = 0;
	double x2 = 0;
	double y1 = 0;
	double y2 = 0;
	double X1[] = { x1, y1, 0 };
	double X2[] = { x2, y2, 0 };
	double Y1[] = { x1, y1, 4 };
	double Y2[] = { x2, y2, 4 };
	//double 

	int i = 0;// будем делать в цикле

	glTranslated(4.5, 2, 0);// верхний полукрунг
	//glRotated(atan(8/3), 1, 1, 0);
	O[0] = 0;
	O[1] = 0;
	O2[0] = 0;
	O2[1] = 0;
	glBegin(GL_TRIANGLES);
	double r = 1.3439;
	for (i = 0; i < 50; i++)
	{

		x1 = cos((180 - (3.60 * i) * 3.14) / 180 + atan(r)) * 4, 27;
		y1 = sin((180 - (3.60 * i) * 3.14) / 180 + atan(r)) * 4, 27;

		X1[0] = x1;
		X1[1] = y1;
		Y1[0] = x1;
		Y1[1] = y1;

		x2 = cos((180 - (3.60 * (i + 1)) * 3.14) / 180 + atan(r)) * 4, 27;
		y2 = sin((180 - (3.60 * (i + 1)) * 3.14) / 180 + atan(r)) * 4, 27;

		X2[0] = x2;
		X2[1] = y2;
		Y2[0] = x2;
		Y2[1] = y2;


		glNormal3d(0, 0, -1);//рисуем нижний полукруг
		glColor3d(0.7, 0.2, 0.2);
		//glNormal3d(0, 0, -1);
		glVertex3dv(X2);
		glVertex3dv(X1);
		glVertex3dv(O);

		
		glNormal3d(0, 0, 1);//рисуем верхний полукруг
		glTranslated(0, 0, 4);
		glColor3d(0.7, 0.2, 0.6);
		//glNormal3d(0, 0, 1);
		glVertex3dv(Y2);
		glVertex3dv(Y1);
		glVertex3dv(O2);
		glTranslated(0, 0, -4);
	}
	glEnd();

	glBegin(GL_QUADS);// полуцилиндрическая поверхность
	x1 = 0;
	x2 = 0;
	y1 = 0;
	y2 = 0;
	r = 1.3439;
	for (i = 0; i < 50; i++)
	{

		x1 = cos((180 - (3.60 * i) * 3.14) / 180 + atan(r)) * 4, 27;
		y1 = sin((180 - (3.60 * i) * 3.14) / 180 + atan(r)) * 4, 27;

		X1[0] = x1;
		X1[1] = y1;
		Y1[0] = x1;
		Y1[1] = y1;

		x2 = cos((180 - (3.60 * (i + 1)) * 3.14) / 180 + atan(r)) * 4, 27;
		y2 = sin((180 - (3.60 * (i + 1)) * 3.14) / 180 + atan(r)) * 4, 27;

		X2[0] = x2;
		X2[1] = y2;
		Y2[0] = x2;
		Y2[1] = y2;

		
		N = VectorN(X1, X2, Y2);
		glNormal3d(N[0], N[1], N[2]);
		glColor3d(0.7, 0.2, 0.2);
		
		glVertex3dv(X2);
		glVertex3dv(X1);
		glVertex3dv(Y1);
		glVertex3dv(Y2);
	}
	//glNormal3d(0, 0, 1);

	glEnd();
	//конец рисования квадратика станкина
		////double A[2] = { -4, -4 };
	////double B[2] = { 4, -4 };
	////double C[2] = { 4, 4 };
	////double D[2] = { -4, 4 };
	//
	//glBindTexture(GL_TEXTURE_2D, texId);
	//
	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);
	//
	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);
	//
	//glEnd();

   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}


//строка 325
std::array<double,3> VectorN(double *N1, double *N2, double *N3) 
{	
	double x1, y1, z1, x2, y2, z2, x3, y3, z3,nx,ny,nz, ax,ay,az,bx,by,bz,n1x,n1y,n1z ;
	//std::array<double, 3> N; 
	//double vectorA[] = {0,0,0};
	//double vectorB[] = { 0,0,0 };
	//double i;
	//double j;
	//double k;

	//double N[] = { 0,0,0 };
	//определяем координаты векторов
	//vectorA[0] = N2[1] - N1[1];
	//vectorA[1] = N2[2] - N1[2];
	//vectorA[2] = N2[3] - N1[3];
	//
	//vectorB[0] = N3[1] - N1[1];
	//vectorB[1] = N3[2] - N1[2];
	//vectorB[2] = N3[3] - N1[3];
	// определяем координаты векторa нормали
	x1 = N1[0];		y1 = N1[1];		z1 = N1[2];
	x2 = N2[0];		y2 = N2[1];		z2 = N2[2];
	x3 = N3[0];		y3 = N3[1];		z3 = N3[3];

	ax = x2 - x1;	ay = y2 - y1;	az = z2 - z1;
	bx = x3 - x1;	by = y3 - y1;	bz = z3 - z1;

	//ax = vectorA[0];
	//ay = vectorA[1];
	//az = vectorA[2];
	//
	//bx = vectorB[0];
	//by = vectorB[1];
	//bz = vectorB[2];

	n1z = ax * by - ay * bx;
	n1y = az * bx - ax * bz;
	n1x = ay * bz - az * by;

	nx = n1x / sqrt(n1x* n1x + n1y* n1y + n1z* n1z);
	ny = n1y / sqrt(n1x * n1x + n1y * n1y + n1z * n1z);
	nz = n1z / sqrt(n1x * n1x + n1y * n1y + n1z * n1z);
	//ny = ny * (-1);
	std::array<double, 3>N = { nx,ny,nz };
	return N;



}