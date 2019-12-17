#include<GL/glut.h>
#include<vector>
#include<algorithm>
#define DEBUG
#ifdef DEBUG
#define dprintf printf
#else
#define dprintf /\
/printf
#endif
using namespace std;


/*------------------绘图算法相关-----------------*/
/*活性边表 多边行扫描转换*/
typedef struct XET {
	float x;
	float dx, ymax;
	XET* next;
} AET, NET;

typedef struct point {
	float x;
	float y;
}point;
bool operator == (point& p1, point p2) {
	return p1.x == p2.x && p1.y == p2.y;
}
typedef vector<point> points;
typedef vector<points> polys;
void drawAET (points& polypoint) {



	int POINTNUM = polypoint.size ();
	/******计算最高点的y坐标(扫描到此结束)****************************************/
	int MaxY = 0;
	int i;
	for (i = 0; i < POINTNUM; i++)
		if (polypoint[i].y > MaxY)
			MaxY = polypoint[i].y;

	/*******初始化AET表***********************************************************/
	AET* pAET = new AET;
	pAET->next = NULL;

	/******初始化NET表************************************************************/
	NET* pNET[1024];
	for (i = 0; i <= MaxY; i++) {
		pNET[i] = new NET;
		pNET[i]->next = NULL;
	}
	glColor3f (1.0, 1.0, 1.0);             //设置直线的颜色红色
	glBegin (GL_POINTS);
	/******扫描并建立NET表*********************************************************/
	for (i = 0; i <= MaxY; i++) {
		for (int j = 0; j < POINTNUM; j++)
			if (polypoint[j].y == i) {
				//一个点跟前面的一个点形成一条线段，跟后面的点也形成线段
				if (polypoint[(j - 1 + POINTNUM) % POINTNUM].y > polypoint[j].y) {
					NET* p = new NET;
					p->x = polypoint[j].x;
					p->ymax = polypoint[(j - 1 + POINTNUM) % POINTNUM].y;
					p->dx = (polypoint[(j - 1 + POINTNUM) % POINTNUM].x - polypoint[j].x) / (polypoint[(j - 1 + POINTNUM) % POINTNUM].y - polypoint[j].y);
					p->next = pNET[i]->next;
					pNET[i]->next = p;
				}
				if (polypoint[(j + 1 + POINTNUM) % POINTNUM].y > polypoint[j].y) {
					NET* p = new NET;
					p->x = polypoint[j].x;
					p->ymax = polypoint[(j + 1 + POINTNUM) % POINTNUM].y;
					p->dx = (polypoint[(j + 1 + POINTNUM) % POINTNUM].x - polypoint[j].x) / (polypoint[(j + 1 + POINTNUM) % POINTNUM].y - polypoint[j].y);
					p->next = pNET[i]->next;
					pNET[i]->next = p;
				}
			}
	}
	/******建立并更新活性边表AET*****************************************************/
	for (i = 0; i <= MaxY; i++) {
		//计算新的交点x,更新AET
		NET* p = pAET->next;
		while (p) {
			p->x = p->x + p->dx;
			p = p->next;
		}
		//更新后新AET先排序*************************************************************/
		//断表排序,不再开辟空间
		AET* tq = pAET;
		p = pAET->next;
		tq->next = NULL;
		while (p) {
			while (tq->next && p->x >= tq->next->x)
				tq = tq->next;
			NET* s = p->next;
			p->next = tq->next;
			tq->next = p;
			p = s;
			tq = pAET;
		}
		//(改进算法)先从AET表中删除ymax==i的结点****************************************/
		AET* q = pAET;
		p = q->next;
		while (p) {
			if (p->ymax == i) {
				q->next = p->next;
				delete p;
				p = q->next;
			} else {
				q = q->next;
				p = q->next;
			}
		}
		//将NET中的新点加入AET,并用插入法按X值递增排序**********************************/
		p = pNET[i]->next;
		q = pAET;
		while (p) {
			while (q->next && p->x >= q->next->x)
				q = q->next;
			NET* s = p->next;
			p->next = q->next;
			q->next = p;
			p = s;
			q = pAET;
		}
		/******配对填充颜色***************************************************************/
		glColor3f (1, 1, 1);
		p = pAET->next;
		while (p && p->next) {
			for (float j = p->x; j <= p->next->x; j++)
				glVertex2i (static_cast<int> (j), i);
			p = p->next->next;//考虑端点情况
		}
	}
	glEnd ();
	glFlush ();
}


/*------------------窗口功能相关-----------------*/
const int SCREENY = 500;
const int SCREENX = 500;
points pointsd = { {250,50},{550,150},{550,400},{250,250},{100,350},{100,100},{120,30} }; //多边形顶点
points temp_poly;
typedef enum state { MOVING_POINT, ADDING_POINT, DRAWING_POLY, DRAWING_DONE } state;
typedef enum addstate { ADD=9999, INSERT, DELETEP} addstate;
typedef enum menufunc { CLEAR, START_MOV_POINT, START_ADD_POINT, START_DEL_POINT };
state stated = DRAWING_POLY;
addstate addstated = ADD;
point* chosed_point = nullptr;
void display (void) {
	glClear (GL_COLOR_BUFFER_BIT);
	glColor3f (1.0, 1.0, 1.0);
	glPointSize (4);
	glBegin (GL_POINTS);
	glClear (GL_COLOR_BUFFER_BIT);        //赋值的窗口显示.
	if (temp_poly.size () >= 3) {
		drawAET (temp_poly);
	}
	glEnd ();


	glBegin (GL_POINTS);
	for (auto i : temp_poly) {
		glVertex2i (i.x, i.y);
	}
	glEnd ();
	glutSwapBuffers ();

}
inline float calDistance (point a1, point a2) {
	return pow ((pow ((a1.x - a2.x), 2) + pow ((a1.y - a2.y), 2)), 0.5);
};
point* getClosetPoint (int xi, int yi) {
	point* temp = nullptr;
	float tempdis = 999999999;
	point click = { xi,yi };
	for (point& i : temp_poly) {
		float temp1 = calDistance (click, i);
		if (temp1 < tempdis) {
			tempdis = temp1;
			temp = &i;
		}
	}
	if (tempdis > 60.0) {
		dprintf ("未选中合适的点\n");
		return nullptr;
	}
	dprintf ("选中了点：(%f, %f)，于位置：%p\n", temp->x, temp->y, temp);
	return temp;
}

void insertPoint (int x, int y) {
	point ti = {x,y};
	point* t1 = getClosetPoint (x, y);
	if (t1 == nullptr) {
		printf ("无法找到合适的插入点\n");
	} else {

	temp_poly.insert (find (temp_poly.begin (), temp_poly.end (), *t1),ti);
	}

}
/*鼠标按键回调*/
void mouseButton (int button, int state, int xi, int yi) {
	float x = xi;
	float y = SCREENY - yi;
	point temp_point = { x,y };
	switch (stated) {
	case MOVING_POINT:
		if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP) {
			if (stated == MOVING_POINT) {
				stated = DRAWING_POLY;
				dprintf ("位置移动结束，当前点新位置(%f, %f)\n", chosed_point->x, chosed_point->y);
			}
		}
		break;
	case ADDING_POINT:
		break;
	case DRAWING_POLY:
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			switch (addstated) {
			case ADD:
				dprintf ("添加点(%f, %f)\n", temp_point.x, temp_point.y);
				temp_poly.emplace_back (temp_point);

				break;
			case INSERT:
				if (temp_poly .size()< 2) {
					printf ("抱歉，当前点数目小于2，无法插入，请您添加更多点，默认选择添加点");
					dprintf ("添加点(%f, %f)\n", temp_point.x, temp_point.y);
					temp_poly.emplace_back (temp_point);
				} else {
					dprintf ("插入点(%f, %f)\n", temp_point.x, temp_point.y);
					insertPoint (temp_point.x, temp_point.y);
				}

				break;
			case DELETEP:
				if (temp_poly.size () < 2) {
					printf ("抱歉，当前点数目小于1，无法删除，请您添加更多点\n");

				} else {
					point* to_be_delete = getClosetPoint (temp_point.x, temp_point.y);
					if (to_be_delete == nullptr) {
						printf ("抱歉，未选中任何点，无法删除\n");
					} else {
						dprintf ("删除点(%f, %f)\n", temp_point.x, temp_point.y);
						temp_poly.erase (find (temp_poly.begin (), temp_poly.end (), *to_be_delete));

					}
				}
				break;
			default:
				break;
			}
		}
		if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
			chosed_point = getClosetPoint (x, y);
			if (chosed_point == nullptr) {
				stated = DRAWING_POLY;
				dprintf ("未选中合适的点\n");
			} else {
				stated = MOVING_POINT;
				dprintf ("位置移动开始，当前点位置(%f, %f)\n", chosed_point->x, chosed_point->y);
			}
		}
		break;
	case DRAWING_DONE:

		break;
	default:
		break;
	}
}
/*鼠标移动回调*/
void mouseMove (int xi, int yi) {
	float x = xi;
	float y = SCREENY - yi;
	switch (stated) {
	case MOVING_POINT:
		dprintf ("当前点位置(%f, %f)， ", chosed_point->x, chosed_point->y);
		chosed_point->x = x;
		chosed_point->y = y;
		dprintf ("当前点新位置(%f, %f)\n", chosed_point->x, chosed_point->y);
		glutPostRedisplay ();
		break;
	case ADDING_POINT:
		break;
	case DRAWING_POLY:
		break;
	case DRAWING_DONE:

		break;
	default:
		break;
	}
}

void menuCtr (int funci) {
	switch (funci) {
	case CLEAR:
		//清空所有多边形
		temp_poly.clear ();
		dprintf ("清空所有记录\n");
		break;
	case ADD:
		addstated = ADD;
		printf ("选择了添加点模式\n");
		break;

	case INSERT:
		addstated = INSERT;
		printf ("选择了插入点模式\n");

		break;

	case DELETEP:
		addstated = DELETEP;
		printf ("选择了删除点模式\n");
		break;
	default:
		break;
	}
	glutPostRedisplay ();
}

int main (int argc, char** argv) {
	glutInit (&argc, argv);							//初始化
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);	//设置显示模式
	glutInitWindowPosition (600, 100);				//设置窗口的顶部和左边位置
	glutInitWindowSize (SCREENX, SCREENY);					//设置窗口的高度和宽度
	glutCreateWindow ("实验四：填充与多边形扫描转换");	//设置窗口标题
	glClearColor (0, 0, 0, 0);							//设置窗口背景颜色
	glMatrixMode (GL_PROJECTION);					//设置窗口视图
	gluOrtho2D (0, SCREENX, 0, SCREENY);
	glutDisplayFunc (display);						//注册显示函数

	/*注册各种自定义函数*/
	//glutTimerFunc (50, &_timeCtr, 1);
	glutMouseFunc (mouseButton);
	glutMotionFunc (mouseMove);

	/*注册右键菜单栏*/
	glutCreateMenu (menuCtr);
	glutAddMenuEntry ("clear", CLEAR);
	glutAddMenuEntry ("add", ADD);
	glutAddMenuEntry ("insert", INSERT);
	glutAddMenuEntry ("delete", DELETEP);
	glutAttachMenu (GLUT_RIGHT_BUTTON);

	glutMainLoop ();								//开始主循环
	return 0;
}