#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>
#include <GL/glut.h>
#include <list>
#include <random>
#include <stdlib.h>
#include <time.h>
#include <condition_variable>

//jak pileczka jest wewnatrz prostokata, inne czekaja az ta z niego wyjdzie
//albo prostokat sie usunie z drogi

//konczyc watek rzucajacy pilki - zrobione

//pilka wewnatrz prostokata ma inaczej wygladac (poswiata)

const double RECTANGLE_SPEED_FACTOR = 0.01;
const double RECTANGLE_SPEED_MIN = 0.003;
const int BALL_THROWING_DELAY = 100;
const double radius = 0.02;

std::mutex cv_m;
std::condition_variable cv;

std::mutex rectangle_mutex;

std::mutex list_mutex;

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

	PlanarVector operator-(PlanarVector& other){
		return PlanarVector(x - other.x, y - other.y);
	}

	PlanarVector& operator-=(PlanarVector& other){
		x -= other.x;
		y -= other.y;
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

	void moveBackwards(){
		position -= velocity;
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

class Rectangle{
public:
	PlanarVector getVelocity(){
		return velocity;
	}

	void freeFromOccupant(){
		occupiedBy = nullptr;
		//std::cout << "End occupation\n";
	}

	Ball* getOccupant(){
		return occupiedBy;
	}

	void occupyBy(Ball* ball){
		//std::cout << "Start occupation\n";
		occupiedBy = ball;
	}

	bool isOpen(){
		return !occupiedBy;
	}

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
	Ball* occupiedBy{};
	Color color;
	PlanarVector leftTopCornerPosition;
	double width{0.1};
	double height{0.1};
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
	double y2 = y1 + height;
	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(color.R, color.G, color.B);
		glVertex3f(x1, y1, 0.0);
		glVertex3f(x2, y1, 0.0);
		glVertex3f(x1, y2, 0.0);
		glVertex3f(x2, y2, 0.0);
    glEnd();
}

bool isBallInsideRectangle(PlanarVector& ballPosition, Rectangle* rectangle){
	PlanarVector topLeftCornerPosition = rectangle->getPosition();
	return ballPosition.getY() + radius > topLeftCornerPosition.getY()
			&& ballPosition.getY() - radius < topLeftCornerPosition.getY() + rectangle->getHeight()
			&& ballPosition.getX() + radius > topLeftCornerPosition.getX()
			&& ballPosition.getX() - radius < topLeftCornerPosition.getX() + rectangle->getWidth();
}

bool isBallInsideRectangle(Ball* ball, Rectangle* rectangle){
	return isBallInsideRectangle(ball->getPosition(), rectangle);
}

bool isMoveAllowed(Ball* ball, Rectangle* rectangle){
	PlanarVector positionAfterMove = ball->getPosition() + ball->getVelocity();
	bool ballInsidePre = rectangle->getOccupant() == ball;
	bool ballInsidePost = isBallInsideRectangle(positionAfterMove, rectangle);
	bool rectangleOccupied = !rectangle->isOpen();
	return ballInsidePre
		|| !ballInsidePost
		|| !rectangleOccupied;
}

void takeCareOfBall(Ball* ball, Rectangle* rectangle){
	while(ball->getBounceCount() < 6 && ball->isActive()){
		{
			std::lock_guard<std::mutex> lk(rectangle_mutex);
			if(isMoveAllowed(ball, rectangle)){
				ball->move();
				std::unique_lock<std::mutex> lk(cv_m);
				if(isBallInsideRectangle(ball, rectangle))
					rectangle->occupyBy(ball);
			}/* else {
				std::unique_lock<std::mutex> lk(cv_m);
				cv.wait(lk, [ball, rectangle]{return isMoveAllowed(ball, rectangle);});
			}*/
		}
		if(!isMoveAllowed(ball, rectangle)){
			std::unique_lock<std::mutex> lk(cv_m);
			cv.wait(lk, [ball, rectangle]{return isMoveAllowed(ball, rectangle);});
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		PlanarVector position = ball->getPosition();
		if(position.getX() < -1.0 || position.getX() > 1.0){
			ball->bounceVerticalWall();
		}
		if(position.getY() < -1.0 || position.getY() > 1.0){
			ball->bounceHorizontalWall();
		}			
	}
	if(rectangle->getOccupant() == ball)
		rectangle->freeFromOccupant();
	ball->deactivate();
}

class BallThread{
private:
	Ball* ball{};
	Rectangle* rectangle{};
	std::thread* thread{};
public:
	BallThread(Ball* ball, Rectangle* rectangle = nullptr)
								: ball(ball), rectangle(rectangle){
		thread = new std::thread(std::ref(takeCareOfBall), ball, rectangle);
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
	static void takeCareOfRectangle(Rectangle* rectangle, bool* keepRunning){
		while(*keepRunning){
			Ball* occupant = rectangle->getOccupant();
			if(occupant){
				if(!isBallInsideRectangle(occupant, rectangle)){
					{
						std::lock_guard<std::mutex> lk(cv_m);
						rectangle->freeFromOccupant();
					}
					cv.notify_all();

				}
			}
			{
				std::unique_lock<std::mutex> listLock(list_mutex, std::try_to_lock);

				if(listLock.owns_lock()){
					std::list<Ball*> endangeredBalls;
					for(BallThread* ballThread : ballThreads){
						if(isEndangeredByRectangle(ballThread->getBall())){
							endangeredBalls.push_back(ballThread->getBall());
						}
					}
					{
						std::lock_guard<std::mutex> lk(cv_m);	
						rectangle->move();
					}
					cv.notify_all();
					for(Ball* ball : endangeredBalls){
						std::lock_guard<std::mutex> lk(rectangle_mutex);
						if(isBallInsideRectangle(ball, rectangle) && ball != rectangle->getOccupant()){
							if(rectangle->isOpen()){
								rectangle->occupyBy(ball);
							} else {
								if(rectangle->getVelocity().getX() > 0)
									ball->getPosition().setX(rectangle->getPosition().getX() + rectangle->getWidth() + radius);
								else
									ball->getPosition().setX(rectangle->getPosition().getX() - radius);
								if(ball->getPosition().getX() > 1.0 || ball->getPosition().getX() < -1.0)
									ball->deactivate();
							}
						}
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			PlanarVector position = rectangle->getPosition();
			if(position.getX() < -1.0 || position.getX() + rectangle->getWidth() > 1.0){
				rectangle->bounceVerticalWall();
			}		
		}
	}

	static bool isEndangeredByRectangle(Ball* ball){
		if(!rectangle)
			return false;
		if(rectangle->getVelocity().getX() > 0)
			return isRightwardsFromRectangle(ball);
		else
			return isLeftwardsFromRectangle(ball);

	}

	static bool isLeftwardsFromRectangle(Ball* ball){
		return ball->getPosition().getX() < rectangle->getPosition().getX();
	}

	static bool isRightwardsFromRectangle(Ball* ball){
		return ball->getPosition().getX() > rectangle->getPosition().getX() + rectangle->getWidth();
	}

    static void displayMe(void){
        glClear(GL_COLOR_BUFFER_BIT);

		if(rectangle){
			PlanarVector position = rectangle->getPosition();
			drawRect(position, rectangle->getWidth(), rectangle->getHeight(), rectangle->getColor());
		}

		{
			std::unique_lock<std::mutex> listLock(list_mutex);
			{
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
        	}
    	}

        glFlush();
    }

    static void addRectangle(Rectangle* rectangle){
    	if(!rectangleThread){
    		running = true;
    		ApplicationState::rectangle = rectangle;
    		rectangleThread = new std::thread(std::ref(ApplicationState::takeCareOfRectangle), rectangle, &running);
    	}
    }

    static void addBall(Ball* ball){
    	ballThreads.push_back(new BallThread(ball, rectangle));
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
		std::this_thread::sleep_for(std::chrono::milliseconds(((rand() % 5) + 2) * BALL_THROWING_DELAY));
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

	Rectangle* rectangle = new Rectangle();
	rectangle->setVelocity(0.003, 0.0);
	Color color(0.5, 0.5, 0.5);
	rectangle->setColor(color);
	rectangle->setWidth(0.6);
	rectangle->setHeight(0.3);
	ApplicationState::addRectangle(rectangle);
	throwingThread = new std::thread(std::ref(keepThrowingBalls));

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
