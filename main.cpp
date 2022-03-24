#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>
#include <GL/glut.h>
#include <list>
#include <random>
#include <stdlib.h>
#include <time.h>  

class PlanarVector{
private:
	double x{};
	double y{};
public:
	PlanarVector(const double x = 0.0, const double y = 0.0) : x(x), y(y){};

	PlanarVector operator+(PlanarVector& other){
		return PlanarVector(x + other.x, y + other.y);
	}

	PlanarVector& operator+=(PlanarVector& other){
		x += other.x;
		y += other.y;
		return *this;
	}

	void setX(double x){
		this->x = x;
	}

	void setY(double y){
		this->y = y;
	}

	double getX(){
		return x;
	}

	double getY(){
		return y;
	}
};

class Color{
public:
	Color(double r = 1.0, double g = 1.0, double b = 1.0) : 
				R(r), G(g), B(b) {};

	double R;
	double G;
	double B;
};

class Ball{
public:
	void setColor(Color& color){
		this->color = Color(color.R, color.G, color.B);
	}

	void setColor(double r, double g, double b){
		this->color = Color(r, g, b);
	}

	Color getColor(){
		return this->color;
	}

	void deactivate(){
		this->active = false;
	}

	bool isActive(){
		return active;
	}

	void move(){
		position += velocity;
	}

	void bounceVerticalWall(){
		velocity.setX(-velocity.getX());
		bouncesCount++;
	}

	void bounceHorizontalWall(){
		velocity.setY(-velocity.getY());
		bouncesCount++;
	}

	int getBounceCount(){
		return bouncesCount;
	};

	PlanarVector& getPosition(){
		return position;
	}

	PlanarVector& getVelocity(){
		return velocity;
	}

	void setPosition(PlanarVector& position){
		setPosition(position.getX(), position.getY());
	}

	void setPosition(double x, double y){
		this->position = PlanarVector(x, y);
	}

	void setVelocity(PlanarVector& velocity){
		setVelocity(velocity.getX(), velocity.getY());
	}

	void setVelocity(double x, double y){
		this->velocity = PlanarVector(x, y);
	}
private:
	Color color;
	bool active{true};
	int bouncesCount{};
	PlanarVector position;
	PlanarVector velocity;
};

void drawCircle(PlanarVector& position, double radius, Color color){
	static double SQRT_2 = sqrt(2.0);
	double x = position.getX();
	double y = position.getY();
	double scaledRadius = radius / SQRT_2;
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(color.R, color.G, color.B);
		glVertex3f(x, y, 0.0);
        glVertex3f(x + radius, y, 0.0);
        glVertex3f(x + scaledRadius, y + scaledRadius, 0.0);
        glVertex3f(x, y + radius, 0.0);
        glVertex3f(x - scaledRadius, y + scaledRadius, 0.0);
        glVertex3f(x - radius, y, 0.0);
        glVertex3f(x - scaledRadius, y - scaledRadius, 0.0);
        glVertex3f(x, y - radius, 0.0);
        glVertex3f(x + scaledRadius, y - scaledRadius, 0.0);
        glVertex3f(x + radius, y, 0.0);
    glEnd();
}

void dupa(Ball* ball){
	while(ball->getBounceCount() < 6){
		ball->move();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		PlanarVector position = ball->getPosition();
		if(position.getX() < -1.0 || position.getX() > 1.0){
			ball->bounceVerticalWall();
		}
		if(position.getY() < -1.0 || position.getY() > 1.0){
			ball->bounceHorizontalWall();
		}			
	}
	ball->deactivate();
}

class ApplicationState{
private:
	static std::list<Ball*> balls;
public:
    static void displayMe(void){
        glClear(GL_COLOR_BUFFER_BIT);
        double radius = 0.02;

        std::list<Ball*>::iterator i = balls.begin();
		while (i != balls.end())
		{
    		bool isActive = (*i)->isActive();
    		if (!isActive)
    		{
    			delete (*i);
        		balls.erase(i++);  // alternatively, i = items.erase(i);
    		}
    		else
    		{
        		drawCircle((*i)->getPosition(), radius, (*i)->getColor());
        		++i;
    		}
		}

        glFlush();
    }

    static void revalidateBalls(){

    }

    static void addBall(Ball* ball){
    	balls.push_back(ball);
		std::thread yetAnotherThread(std::ref(dupa), ball);
		yetAnotherThread.detach();
    }
};

std::list<Ball*> ApplicationState::balls = std::list<Ball*>();

void forceRefresh(int data){
	glutTimerFunc(20, forceRefresh, -1);
	glutPostRedisplay();
}


PlanarVector getRandomVelocity(){
	double lower_bound = 0.003;
   	double upper_bound = 0.01;
   	static std::uniform_real_distribution<double> unif_y(lower_bound,upper_bound);
   	static std::uniform_real_distribution<double> unif_x(-upper_bound,upper_bound);
   	static std::default_random_engine re;
   	double x = unif_x(re);
   	double y = unif_y(re);
   	return PlanarVector(x, y);
}

double getRandom(){
	double lower_bound = 0.0;
   	double upper_bound = 1.0;
   	static std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
   	static std::default_random_engine re;
   	return unif(re);
}

Color randomColor(){
	return Color(getRandom(), getRandom(), getRandom());
}

void keepThrowingBalls(){
	while(true){
		Ball* ball = new Ball();
		ball->setPosition(0.0, -1.0);
		Color color = randomColor();
		ball->setColor(color);
		PlanarVector velocity = getRandomVelocity();
		ball->setVelocity(velocity);
		ApplicationState::addBall(ball);
		std::this_thread::sleep_for(std::chrono::milliseconds(((rand() % 5) + 2) * 500));
	}
}
 
int main(int argc, char** argv)
{
	srand(time(NULL));

	std::thread yetAnotherThread(std::ref(keepThrowingBalls));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Hello world!");
    glutDisplayFunc(ApplicationState::displayMe);
    glutTimerFunc(20, forceRefresh, -1);
    glutMainLoop();
    return 0;
}
