# include "iGraphics.h"



#define PI 3.1416

#define SCREENWIDTH 500
#define SCREENHEIGHT 780

#define perTeam 5

#define minDr 15.0
#define vMin 0.1

#define MAXDEVMETER 10



typedef struct
{
	double p[2];
	double v[2];
	double m;
	double uk;
	double r;
	double color[3];
}
object;





const double MAPBORDER[2][4] = {{30, 198, SCREENWIDTH - 197, SCREENWIDTH - 30},
						{40, 85, SCREENHEIGHT - 92, SCREENHEIGHT - 50}};

const double CENTER[2] = {(MAPBORDER[0][1] + MAPBORDER[0][2]) / 2, (MAPBORDER[1][1] + MAPBORDER[1][2]) / 2};

double poly[2][12] = {{MAPBORDER[0][0], MAPBORDER[0][0], MAPBORDER[0][1], MAPBORDER[0][1], 
						MAPBORDER[0][2], MAPBORDER[0][2], MAPBORDER[0][3], MAPBORDER[0][3],
						MAPBORDER[0][2], MAPBORDER[0][2], MAPBORDER[0][1], MAPBORDER[0][1],},
					{MAPBORDER[1][1], MAPBORDER[1][2], MAPBORDER[1][2], MAPBORDER[1][3], 
						MAPBORDER[1][3], MAPBORDER[1][2], MAPBORDER[1][2], MAPBORDER[1][1], 
						MAPBORDER[1][1], MAPBORDER[1][0], MAPBORDER[1][0], MAPBORDER[1][1]}};

const double ARROW[2][7] = {{50, 50, 70, 50, 50, 0, 0}, {8, 20, 0, -20, -8, -8, 8}};

const double FORMATION[][5][2] = {{{0, 70}, {55, 30}, {-55, 30}, {0, 150}, {0, 275}},
									{{0, 60}, {100, 140}, {-100, 140}, {0, 160}, {0, 275}},
									{{35, 110}, {-35, 110}, {100, 80}, {-100, 80}, {0, 275}},
									{{-140, 70}, {-50, 90}, {50, 110}, {140, 130}, {0, 275}}};


char ballImgAddress[] = "Images/Ball/_.bmp";
char redScoreAddress[] = "Images/Scores/Red_0.bmp";
char blueScoreAddress[] = "Images/Scores/Blue_0.bmp";


object objects[(perTeam*2)+1];

int selectedPlayer = 0;
double powerX = 0, powerY = 0;
double Pmax = 120;

int movingSide = 1;
int moveEnd = 1;

double deviationScale[2] = {1, 1};
double deviationLimit;
int deviationMeter = 0;
int deviationNext = 1;


int goalNum[2] = {0};
int winGoal = 3;

int showMenu = 1;
int showGoalPopUp = 0;



void setPlayerPositions(int a, int b)
{
	// Blue Team
	for (int i = 1; i <= perTeam; i++)
	{
		objects[i].p[0] = CENTER[0] + FORMATION[a][i-1][0];
		objects[i].p[1] = CENTER[1] - FORMATION[a][i-1][1];
	}

	// Red Team
	for (int i = perTeam+1; i <= 2*perTeam; i++)
	{
		objects[i].p[0] = CENTER[0] + FORMATION[b][i-1-perTeam][0];
		objects[i].p[1] = CENTER[1] + FORMATION[b][i-1-perTeam][1];
	}
}

void spawnObjects()
{
	// set color
	objects[0].color[0] = 130; // 150
	objects[0].color[1] = 65; // 75
	objects[0].color[2] = 0; // 0
	for (int i = 1; i <= perTeam; i++)
	{
		objects[i].color[0] = 0;
		objects[i].color[1] = 0;
		objects[i].color[2] = 255;
	}
	for (int i = perTeam+1; i <= 2*perTeam; i++)
	{
		objects[i].color[0] = 225;
		objects[i].color[1] = 0;
		objects[i].color[2] = 0;
	}

	// set radius, uk, mass
	objects[0].r = 10;
	objects[0].m = 25; // Subject to chng
	objects[0].uk = 0.16; // Subject to chng
	for (int i = 1; i <= 2*perTeam; i++)
	{
		objects[i].r = 15;
		objects[i].m = 50; // Subject to chng
		objects[i].uk = 0.18; // Subject to chng
	}

	// set v
	for (int i = 0; i <= 2*perTeam; i++)
	{
		objects[i].v[0] = 0;
		objects[i].v[1] = 0;
	}

	// set position
	objects[0].p[0] = CENTER[0];
	objects[0].p[1] = CENTER[1];
	setPlayerPositions(3, 2);
}


void initializeMatch()
{
	goalNum[0] = 0;
	goalNum[1] = 0;

	spawnObjects();

	iResumeTimer(0);

	showMenu = 0;
}


double measureDistance(double x1, double y1, double x2, double y2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double d = sqrt((dx*dx) + (dy*dy));
	return d;
}

int isMoving()
{
	for (int i = 0; i <= 2*perTeam; i++)
	{
		double v[2] = {objects[i].v[0], objects[i].v[1]};
		double vr2 = (v[0]*v[0]) + (v[1]*v[1]);
		if (vr2 > vMin*vMin) return 1;
	}
	return 0;
}

int isMovePossible()
{
	double d = sqrt(powerX*powerX + powerY*powerY);
	if (showMenu == 0 && moveEnd == 1 && d > minDr &&
		selectedPlayer > (movingSide-1)*perTeam && selectedPlayer <= movingSide*perTeam)
	{
		return 1;
	}

	else return 0;
}


int selectObject(int x, int y)
{
	for (int i = 1; i <= perTeam*2; i++)
	{
		double d = measureDistance(x, y, objects[i].p[0], objects[i].p[1]);
		if (d <= objects[i].r) return i;
	}

	return 0;
}


void addVelocity()
{
	int p = selectedPlayer;
	double vk = 0.1;

	double dx = powerX * deviationScale[0];
	double dy = powerY * deviationScale[1];
	double d = sqrt(dx*dx + dy*dy);

	if (d > Pmax)
	{
		dx = ((dx * Pmax) / (d));
		dy = ((dy * Pmax) / (d));
	}

	objects[p].v[0] = dx * vk;
	objects[p].v[1] = dy * vk;

	moveEnd = 0;

	// printf("P --> %f %f\n", powerX, powerY);
	// printf("v --> %f %f\n", objects[p].v[0], objects[p].v[1]);
}


void addDeviation()
{
	double d = sqrt(powerX*powerX + powerY*powerY);
	deviationLimit = d / 4.0;
	if (deviationLimit > 30) deviationLimit = 30;

	if (abs(deviationMeter) == MAXDEVMETER) deviationNext *= -1;
	deviationMeter += deviationNext;

	double theta = ((deviationLimit * deviationMeter) / MAXDEVMETER) * (PI/180);
	double dx = cos(theta);
	double dy = sin(theta);

	deviationScale[0] = ((powerX*dx) - (powerY*dy)) / powerX;
	deviationScale[1] = ((powerX*dy) + (powerY*dx)) / powerY;
}


void addFriction(object* p)
{
	double vx = p->v[0];
	double vy = p->v[1];
	double v = sqrt((vx*vx) + (vy*vy));
	double ratio[2];
	ratio[0] = vx/v;
	ratio[1] = vy/v;

	for (int i = 0; i < 2; i++) p->v[i] -= ratio[i] * p->uk;
}

void moveObjects()
{
	for (int i = 0; i <= perTeam*2; i++)
	{
		double v[2] = {objects[i].v[0], objects[i].v[1]};
		double vr2 = (v[0]*v[0]) + (v[1]*v[1]);
		if (vr2 > vMin*vMin)
		{
			for (int j = 0; j < 2; j++) objects[i].p[j] += objects[i].v[j];
			addFriction(&objects[i]);
		}
	}
}


void swapMove()
{
	if (isMoving() == 0)
	{
		moveEnd = 1;
		movingSide = (movingSide % 2) + 1;
	}
}


void collisionWithBorder()
{
	for (int i = 0; i <= perTeam*2; i++)
	{
		double p[2];
		p[0] = objects[i].p[0];
		p[1] = objects[i].p[1];
		double r = objects[i].r;

		// Horizontal
		if (p[1]-r < MAPBORDER[1][2] && p[1]+r > MAPBORDER[1][1])
		{
			if (p[0] - r < MAPBORDER[0][0])
			{
				objects[i].p[0] = MAPBORDER[0][0] + r;
				objects[i].v[0] *= -1;
			}

			else if (p[0] + r > MAPBORDER[0][3])
			{
				objects[i].p[0] = MAPBORDER[0][3] - r;
				objects[i].v[0] *= -1;
			}
		}

		else if (p[1] < MAPBORDER[1][3] && p[1] > MAPBORDER[1][0])
		{
			if (p[0] - r < MAPBORDER[0][1])
			{
				objects[i].p[0] = MAPBORDER[0][1] + r;
				objects[i].v[0] *= -1;
			}

			else if (p[0] + r > MAPBORDER[0][2])
			{
				objects[i].p[0] = MAPBORDER[0][2] - r;
				objects[i].v[0] *= -1;
			}
		}


		// Vertical
		if (p[0] < MAPBORDER[0][2] && p[0] > MAPBORDER[0][1])
		{
			if (p[1] - r < MAPBORDER[1][0])
			{
				objects[i].p[1] = MAPBORDER[1][0] + r;
				objects[i].v[1] *= -1;
			}

			else if (p[1] + r > MAPBORDER[1][3])
			{
				objects[i].p[1] = MAPBORDER[1][3] - r;
				objects[i].v[1] *= -1;
			}
		}

		else if (p[0] < MAPBORDER[0][3] && p[0] > MAPBORDER[0][0])
		{
			if (p[1] - r < MAPBORDER[1][1])
			{
				objects[i].p[1] = MAPBORDER[1][1] + r;
				objects[i].v[1] *= -1;
			}

			else if (p[1] + r > MAPBORDER[1][2])
			{
				objects[i].p[1] = MAPBORDER[1][2] - r;
				objects[i].v[1] *= -1;
			}
		}
	}
}


void afterCollisionPosition(object* player1, object* player2, double d)
{
	object p1 = *player1;
	object p2 = *player2;

	double D = p1.r + p2.r;
	double dc = D - d;
	double dx = p2.p[0] - p1.p[0];
	double dy = p2.p[1] - p1.p[1];

	dx = ((dx * dc) / (2 * d)) + 0.2;
	dy = ((dy * dc) / (2 * d)) + 0.2;

	(*player1).p[0] -= dx;
	(*player1).p[1] -= dy;
	(*player2).p[0] += dx;
	(*player2).p[1] += dy;
}

void afterCollisionVelocity(object* player1, object* player2)
{
	object p1 = *player1;
	object p2 = *player2;
	double m1 = p1.m;
	double m2 = p2.m;
	double dx = p2.p[0] - p1.p[0];
	double dy = p2.p[1] - p1.p[1];
	double vx1 = p1.v[0];
	double vy1 = p1.v[1];
	double vx2 = p2.v[0];
	double vy2 = p2.v[1];

	double vk1 = (vx1*dx + vy1*dy) / (dx*dx + dy*dy);
	double vk2 = (vx2*dx + vy2*dy) / (dx*dx + dy*dy);

	double v1[2];
	double v2[2];
	v1[0] = dx * vk1;
	v1[1] = dy * vk1;
	v2[0] = dx * vk2;
	v2[1] = dy * vk2;

	for (int i = 0; i < 2; i++)
	{
		(*player1).v[i] -= v1[i];
		(*player2).v[i] -= v2[i];

		(*player1).v[i] += ((m1-m2)*v1[i] + 2*m2*v2[i]) / (m1+m2);
		(*player2).v[i] += ((m2-m1)*v2[i] + 2*m1*v1[i]) / (m1+m2);
		// printf("%d, %d, %d --- %lf  %lf\n", p1, p2, i, p1.v[i], p2.v[i]);
	}
}

void collisionWithObject()
{
	for (int i = 0; i <= perTeam*2; i++)
	{
		for (int j = i+1; j <= perTeam*2; j++)
		{
			int d = measureDistance(objects[i].p[0], objects[i].p[1], objects[j].p[0], objects[j].p[1]);
			if (d < objects[i].r + objects[j].r)
			{
				afterCollisionPosition(&objects[i], &objects[j], d);
				afterCollisionVelocity(&objects[i], &objects[j]);
			}
		}
	}
}


void goalCelebration()
{
	iPauseTimer(1);
	
	if (showGoalPopUp == 3 || showGoalPopUp == 4)
	{
		showMenu = 1;
		iPauseTimer(0);
	}

	showGoalPopUp = 0;
	spawnObjects();
}


int isInGoalSpace(double* p)
{
	if (p[1] > MAPBORDER[1][2]) return 1;

	else if (p[1] < MAPBORDER[1][1]) return 2;

	else return 0;
}

void isGoal()
{
	if (showGoalPopUp == 0)
	{
		showGoalPopUp = isInGoalSpace(objects[0].p);
		if (showGoalPopUp != 0)
		{
			iResumeTimer(1);
			switch (showGoalPopUp)
			{
			case 1:
				goalNum[0]++;
				// printf("Blue Scores!\n");
				break;

			case 2:
				goalNum[1]++;
				// printf("Red Scores!\n");
				break;
			
			default:
				break;
			}

			// Win
			if (goalNum[showGoalPopUp-1] == winGoal) showGoalPopUp += 2;
		}
	}
}


void drawBall()
{
	double x = objects->p[0];
	double y = objects->p[1];
	/*
	int i = (int)(x / 6) % 3;
	int j = (int)(y / 9) % 2;
	i = i % 3;
	j = j % 2;
	*/
	int idx = (int)(x / 6) % 3 + ((int)(y / 9) % 2)*3;

	ballImgAddress[12] = idx + '1';

	iShowBMP2(x - 11, y - 11, ballImgAddress, 0x00FF00);
}

void drawPlayers()
{
	for (int i = 1; i < (2*perTeam)+1; i++)
	{
		iSetColor(objects[i].color[0], objects[i].color[1], objects[i].color[2]);
		iFilledCircle(objects[i].p[0], objects[i].p[1], objects[i].r);

		iSetColor(255, 255, 255);
		iFilledCircle(objects[i].p[0], objects[i].p[1], (11*objects[i].r)/20, ((i+4)/5)*4);

		iSetColor(0, 0 ,0);
		for (double j = 0; j < .6; j+=.1) iCircle(objects[i].p[0], objects[i].p[1], objects[i].r+j);
	}
}

void drawArrow(double* p)
{
	if (isMovePossible() == 0) return;

	double dx = powerX * deviationScale[0];
	double dy = powerY * deviationScale[1];
	double d = sqrt(dx*dx + dy*dy);

	double newArrow[2][7];
	double x, y;
	double scale = ((d / Pmax) * 0.75) + 0.25;
	if (scale > 1) scale = 1;

	dx = dx / d;
	dy = dy / d;

	for (int i = 0; i < 7; i++)
	{
		// Rotate
		x = ARROW[0][i];
		y = ARROW[1][i];
		newArrow[0][i] = (x*dx) - (y*dy);
		newArrow[1][i] = (x*dy) + (y*dx);

		// Scaling
		newArrow[0][i] *= scale;
		newArrow[1][i] *= scale;

		// Shift
		newArrow[0][i] += p[0];
		newArrow[1][i] += p[1];
	}
	// printf("%f, %f\n", newArrow[2][0], newArrow[2][1]);
	iSetColor(80, 216, 255); // 150 100 150
	iFilledPolygon(newArrow[0], newArrow[1], 7);
}

void showGoal()
{
	blueScoreAddress[19] = goalNum[0] + '0';
	// printf("%s\n", blueScoreAddress);
	iShowBMP(140, 30, blueScoreAddress);
	
	redScoreAddress[18] = goalNum[1] + '0';
	// printf("%s\n", redScoreAddress);
	iShowBMP(130, 697, redScoreAddress);
}


void showFormationPopup()
{
	iShowBMP2(10, 100, "Images/Formation.bmp", 0x0000FF);
}



void testForMenu()
{
	/*
	Start - 250, 520
	Settings - 100, 415 -- 25
	Controls - 400, 415 -- 25
	Exit - 255, 220
	*/

	iSetColor(255, 0, 0);
	iCircle(105, 415, 30); // Settings
	iCircle(402, 415, 30); // Controls
	iEllipse(252, 520, 55, 22); // Play
	iEllipse(254, 222, 57, 20); // Exit
}





void perFrame()
{
	// clock_t start1, end;
    // double cpu_time_used;
    // start1 = clock();


	if (selectedPlayer != 0) addDeviation();

	if (showMenu == 0)
	{
		moveObjects();
		collisionWithBorder();
		collisionWithObject();
		isGoal();
	}

	if (moveEnd == 0) swapMove();



	// end = clock();
    // cpu_time_used = ((double) (end - start1)) / CLOCKS_PER_SEC;
    // printf("Execution time: %f seconds\n", cpu_time_used);
}



void iDraw()
{
	iClear();

	// Field
	iShowBMP(0, 30, "Images/Field.bmp");

	// Drawing Map Border
	iSetColor(255, 0, 0);
	iPolygon(poly[0], poly[1], 12);

	// Show goals
	showGoal();

	// Draw Arrow
	drawArrow(objects[selectedPlayer].p);

	// Draw Ball
	// iShowBMP2(objects[0].p[0]-10, objects[0].p[1]-10, "Images/BallNew.bmp", 0x00FF00);
	drawBall();

	// Draw Objects
	drawPlayers();

	// Show Menu
	if (showMenu != 0) iShowBMP(0, 5, "Images/MainMenu.bmp");
	// testForMenu();

	// Show Formation
	// showFormationPopup();

	// Goal Celebration Pop up
	if (showGoalPopUp == 1) iShowBMP(CENTER[0] - 125, CENTER[1] - 125, "Images/BlueScores.bmp");
	if (showGoalPopUp == 2) iShowBMP(CENTER[0] - 125, CENTER[1] - 125, "Images/RedScores.bmp");

	// Win Pop Up
	if (showGoalPopUp == 3) iShowBMP(CENTER[0] - 125, CENTER[1] - 125, "Images/BlueWins.bmp");
	if (showGoalPopUp == 4) iShowBMP(CENTER[0] - 125, CENTER[1] - 125, "Images/RedWins.bmp");
	
}

void iMouse(int button, int state, int mx, int my)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		selectedPlayer = selectObject(mx, my);
		// printf("Selected: %d\n", selectedPlayer);

		if (showMenu == 1)
		{
			// Play
			if (((mx-252.0)/57.0)*((mx-252.0)/57.0) + ((my-520.0)/22.0)*((my-520.0)/22.0) < 1)
			{
				initializeMatch();
			}

			// Exit
			else if (((mx-254.0)/57.0)*((mx-254.0)/57.0) + ((my-222.0)/20.0)*((my-222.0)/20.0) < 1)
			{
				exit(0);
			}
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		if (isMovePossible() == 1)
		{
			// powerX -= minDr * (powerX / d); // Subtracting min condition to start from 0
			// powerY -= minDr * (powerY / d);
			addVelocity();
		}
		powerX = 0;
		powerY = 0;
		selectedPlayer = 0;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		selectedPlayer = 0;
	}
}

void iMouseMove(int mx, int my)
{
	int p = selectedPlayer;
	if (p != 0)
	{
		powerX = objects[p].p[0] - mx;
		powerY = objects[p].p[1] - my;

		//double d = measureDistance(objects[n].p[0], objects[n].p[1], mx, my);
		// printf("Power: %f\n", d);
	}
}

void iKeyboard(unsigned char key)
{

}

void iSpecialKeyboard(unsigned char key)
{

}


int main()
{
	iSetTimer(15, perFrame); // iA0
	iPauseTimer(0);

	iSetTimer(4000, goalCelebration); // iA1
	iPauseTimer(1);

	iInitialize(SCREENWIDTH, SCREENHEIGHT, "2D Football");
	return 0;
}
