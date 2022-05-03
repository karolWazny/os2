#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>
#include <GL/glut.h>
#include <list>
#include <random>
#include <stdlib.h>
#include <time.h>

//jak pileczka jest wewnatrz prostokata, inne czekaja az ta z niego wyjdzie
//albo prostokat sie usunie z drogi

//konczyc watek rzucajacy pilki

const double RECTANGLE_SPEED_FACTOR = 0.03;
const double RECTANGLE_SPEED_MIN = 0.003;

double getRandom(){
	double lower_bound = 0.0;
   	double upper_bound = 1.0;
   	static std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
   	static std::default_random_engine re;
   	return unif(re);
}

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

class Rectangle{
public:
	void move(){
		leftTopCornerPosition += velocity;
	}

	void bounceVerticalWall(){
		bool wasRightward = velocity.getX() > 0.0;
		double speed = RECTANGLE_SPEED_MIN + getRandom() * RECTANGLE_SPEED_FACTOR;
		if(wasRightward){
			leftTopCornerPosition.setX(1.0 - width);
			speed *= -1;
		} else {
			leftTopCornerPosition.setX(-1.0);
		}
		velocity.setX(speed);
	}

	void setVelocity(PlanarVector& value){
		setVelocity(value.getX(), value.getY());
	}

	void setVelocity(double x, double y){
		velocity = PlanarVector(x, y);
	}

	void setPosition(double x, double y){
		leftTopCornerPosition.setX(x);
		leftTopCornerPosition.setY(y);
	}

	void setPosition(PlanarVector& position){
		setPosition(position.getX(), position.getY());
	}

	PlanarVector getPosition(){
		return leftTopCornerPosition;
	}

	void setSize(double width, double height){
		this->width = width;
		this->height = height;
	}

	double getWidth(){
		return width;
	}

	void setWidth(double width){
		this->width = width;
	}

	double getHeight(){
		return height;
	}

	void setHeight(double height){
		this->height = height;
	}

	void setColor(Color color){
		this->color = color;
	}

	Color getColor(){
		return color;
	}
private:
	Color color;
	PlanarVector leftTopCornerPosition;
	double width{0.1};
	double height{0.1};
	PlanarVector velocity;
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

void drawRect(PlanarVector& position, double width, double height, Color color = Color(0.5, 0.5, 0.5)){
	double x1 = position.getX();
	double y1 = position.getY();
	double x2 = x1 + width;
	double y2 = y1 - height;
	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(color.R, color.G, color.B);
		glVertex3f(x1, y1, 0.0);
		glVertex3f(x2, y1, 0.0);
		glVertex3f(x1, y2, 0.0);
		glVertex3f(x2, y2, 0.0);
    glEnd();
}

void takeCareOfBall(Ball* ball){
	while(ball->getBounceCount() < 6 && ball->isActive()){
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

void takeCareOfRectangle(Rectangle* rectangle, bool* keepRunning){
	while(*keepRunning){
		rectangle->move();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		PlanarVector position = rectangle->getPosition();
		if(position.getX() < -1.0 || position.getX() + rectangle->getWidth() > 1.0){
			rectangle->bounceVerticalWall();
		}		
	}
}

class BallThread{
private:
	Ball* ball{};
	std::thread* thread{};
public:
	BallThread(Ball* ball) : ball(ball){
		thread = new std::thread(std::ref(takeCareOfBall), ball);
	}

	~BallThread(){
		join();
		delete ball;
		delete thread;
	}

	bool isRunning(){
		return ball->isActive();
	}

	void finish(){
		ball->deactivate();
	}

	void join(){
		if(thread->joinable())
			thread->join();
	}

	Ball* getBall(){
		return ball;
	}
};

class ApplicationState{
private:
	static std::list<BallThread*> ballThreads;
	static std::list<Ball*> balls;
	static Rectangle* rectangle;
	static std::thread* rectangleThread;
	static bool running;
public:
    static void displayMe(void){
        glClear(GL_COLOR_BUFFER_BIT);
        double radius = 0.02;
        
		if(rectangle){
			PlanarVector position = rectangle->getPosition();
			drawRect(position, rectangle->getWidth(), rectangle->getHeight(), rectangle->getColor());
		}

        std::list<BallThread*>::iterator i = ballThreads.begin();
		while (i != ballThreads.end())
		{
    		bool isActive = (*i)->isRunning();
    		if (!isActive)
    		{
    			delete (*i);
        		ballThreads.erase(i++);  // alternatively, i = items.erase(i);
    		}
    		else
    		{
        		drawCircle((*i)->getBall()->getPosition(), radius, (*i)->getBall()->getColor());
        		++i;
    		}
		}

        glFlush();
    }

    static void addRectangle(Rectangle* rectangle){
    	if(!rectangleThread){
    		running = true;
    		ApplicationState::rectangle = rectangle;
    		rectangleThread = new std::thread(std::ref(takeCareOfRectangle), rectangle, &running);
    	}
    }

    static void addBall(Ball* ball){
    	ballThreads.push_back(new BallThread(ball));
    }

    static void finishThreads(){
    	for(BallThread* thread : ballThreads){
    		thread->finish();
    		thread->join();
    		delete thread;
    	}
    	ballThreads = std::list<BallThread*>();
    	running = false;
    		if(rectangleThread){
    		rectangleThread->join();
    		delete rectangleThread;
    		rectangleThread = nullptr;
    		delete rectangle;
    		rectangle = nullptr;
    	}
    }
};

std::list<BallThread*> ApplicationState::ballThreads = std::list<BallThread*>();
Rectangle* ApplicationState::rectangle = nullptr;
std::thread* ApplicationState::rectangleThread = nullptr;
bool ApplicationState::running = true;

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

Color randomColor(){
	return Color(getRandom(), getRandom(), getRandom());
}

bool keepThrowing = false;
std::thread* throwingThread = nullptr;

void keepThrowingBalls(){
	keepThrowing = true;
	while(keepThrowing){
		Ball* ball = new Ball();
		ball->setPosition(0.0, -1.0);
		Color color = randomColor();
		ball->setColor(color);
		PlanarVector velocity = getRandomVelocity();
		ball->setVelocity(velocity);
		ApplicationState::addBall(ball);
		std::this_thread::sleep_for(std::chrono::milliseconds(((rand() % 5) + 2) * 500));
	}
	std::cout << "Not throwing no more!\n";
}

void cleanup(){
	std::cout << "cleanup!\n";
	keepThrowing = false;
	throwingThread->join();
	ApplicationState::finishThreads();
}

void keyboardCallback(unsigned char key, int x, int y){
	if(key == 27) //pressed ESCAPE
	{
		std::cout << "Pressed ESC.\nCleaning up and calling exit...\n";
		exit(0);
	}
}
 
int main(int argc, char** argv)
{
	srand(time(NULL));

	throwingThread = new std::thread(std::ref(keepThrowingBalls));
	Rectangle* rectangle = new Rectangle();
	rectangle->setVelocity(0.003, 0.0);
	Color color(0.5, 0.5, 0.5);
	rectangle->setColor(color);
	rectangle->setWidth(0.4);
	rectangle->setHeight(0.2);
	ApplicationState::addRectangle(rectangle);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Hello world!");
    glutDisplayFunc(ApplicationState::displayMe);
    glutTimerFunc(20, forceRefresh, -1);
    atexit(cleanup);
    glutKeyboardFunc(keyboardCallback);
    glutMainLoop();
    return 0;
}
