#include <GL/glut.h>

class ApplicationState{
public:
    static void displayMe(void){
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_POLYGON);
            //glVertex3f(0.5, 0.0, 0.5);
            glVertex3f(0.5, 0.0, 0.0);
            glVertex3f(0.0, 0.5, 0.0);
            glVertex3f(0.0, 0.0, 0.0);
        glEnd();
        glFlush();
    }
};
 

 
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Hello world!");
    glutDisplayFunc(ApplicationState::displayMe);
    glutMainLoop();
    return 0;
}

