#include <errno.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef __APPLE__ 
#include <GLUT/glut.h>
#else 
#include <GL/glut.h>
#endif

#define TRUE               	1
#define FALSE              	0
#define  X                 	0
#define  Y                 	1
#define  MIN_X            	-399.0 
#define  MAX_X             	400.0
#define  MIN_Y    		-299.0
#define  MAX_Y  		300.0
#define  STRAIGHT		0				/* Straight direction for saucers */
#define  UP			1				/* Up direction for saucers */
#define	 STRAIGHT_X 		1                		/* X value of horizontal vector */
#define  STRAIGHT_Y		0				/* Y value of horizontal vector */
#define  DIAGONAL		1/sqrt(2.0)      		/* X,Y value of diagonal vector */
#define	 MAX_DISTANCE	   	240				/* Max distance bad guy goes before turns */
#define  SHIP_VERTEX       	14				/* Number of vertices a ship has */
#define  SAUCER_VERTEX	   	14				/* Number of vertices a saucer has */
#define  SHIP_TIP          	25.0				/* Y location of the tip of ship */
#define  ROCK_VERTEX       	16				/* Max. number of vertices for each rock */
#define  TORPEDO_VERTEX    	2				/* Number of vertices for a torpedo */
#define  MAX_ROCKS         	140              		/* Maximum number of rocks */
#define  COORD             	2                		/* Number of coords per vertex */
#define	 BIG_VERTEX		24				/* Approximate radius of large bad guy */
#define  SMALL_VERTEX	   	18				/* Approximate radius of small bad guy */
#define  MAX_SHAPES        	10               		/* Maximium number of shapes */
#define  BIG               	1                		/* Big asteroid */
#define  MED               	2                		/* Medium asteroid */
#define  SML               	3                		/* Small asteroid */
#define  BIG_MAX_SPEED     	2.0				/* Maximum speed for a big asteroid */
#define  MED_MAX_SPEED     	3.0				/* Maximum speed for a medium asteroid */
#define  SML_MAX_SPEED     	4.0				/* Maximum speed for a small asteroid */
#define  TORPEDO_SPEED     	10.0				/* Speed of torpedo relative to ship */
#define  MORE_BAD_GUYS	   	1000				/* More saucers if 1000pts away from 10000 */
#define  SMALL_WAIT        	80.0				/* Min time small saucer waits before entering */
#define  LARGE_WAIT        	45.0				/* Min time big saucer waits before entering*/
#define  HIGH_DECREASE     	0.3				/* Rate wait time decreases if near 10000pts */
#define  NORMAL_DECREASE   	0.6				/* Rate that wait time normally decreases */
#define  MAX_AST           	2				/* Number of large asteroids at beginning of each level */
#define  MIN_WAIT          	5				/* Minimum time before ship reappears after dying */
#define  TORPEDO_MIN_SPEED 	6				/* Minimum speed of torpedo */
#define  TORPEDO_FRAMES    	46				/* Frames of torpedo shot by good guy  */
#define	 BAD_SHOT		26				/* Frames of torpedo shot by bad guy */
#define  BAD_SPEED         	2.0              		/* Speed of bad guys */
#define  EXPLOSION_FRAMES  	46				/* Frames of torpedo that triggers explosion */
#define  SHIP_SPEED        	15.0             		/* Maximum speed for the ship */
#define  DIRECTION         	128			    	/* Constant value used for assigning random angular direction */
#define  DEGREES           	360				/* Number of degrees per circle */
#define  DEG_PER_FRAME     	6				/* Number of degrees ship is able to turn each frame */
#define  DEG_RAD_CONV      	M_PI/180.0			/* Constant for determining radian value from angle in degrees */
#define  LINEAR_ACCEL      	0.25            	 	/* linear acceleration model */
#define  DRAG_FACTOR       	0.015				/* Amount of resistance to acceleration */
#define  MAX_TORPEDOES     	4				/* Maximum number of simultaneous torpedoes  */
#define  MAX_EXPLOSIONS    	12				/* Maximum number of simultaneous explosions */
#define  TORPEDO_STREAK    	20.0				/* Length of streak behind each torpedo */
#define  TORPEDO_SIZE      	3.0				/* Size of torpedo */
#define  NUM_POINTS        	10				/* Number of points in an explosion */
#define  SAFE_LIMIT        	128.0				/* Distance all bad guys must be away from good guy before respawning */
#define  SAFE_WIDTH        	50				/* Safe distance from edge of screen for asteroids to spawn */
#define  FRAME_WAIT        	50				/* Frame wait of beginning of each round */
#define  WRECK_WAIT        	64				/* Frame wait of ship after being destroyed */
#define  HYPER_WAIT        	64				/* Frame wait of ship after hyperspace */
#define  EXTRA_SHIP        	10000				/* Number of points needed to earn another ship */
#define  ROUNDS            	6				/* Max number of rounds that difficulty can increase after */
#define  SCORE_STR         	6				/* Maximum number of digits in score */
#define  EXP_ON_REENTRY    	5				/* Odds of explosion on reentry after hyperspace equal to 1/EXP_ON_REENTRY */
#define  MAX_SCORE         	99999				/* Max score */
#define  SMALL             	0.0000001			/* Smallest positive nonzero value considered as nonzero when calculating intersections */
#define  MSEC              	20				/* Number of milliseconds between each frame */

typedef struct {						// Data type for the ship 
	GLfloat ship[SHIP_VERTEX][COORD] ;			// Vertices of ship
	GLfloat normal[SHIP_VERTEX -1][COORD] ;	    		// Normal value for each line segment	
	GLint   frames[SHIP_VERTEX -1] ;			// Array used for storing explosion frames of ship
	GLfloat orientation[COORD] ;                		// Orientation of the ship 
	GLfloat translation[COORD] ;                		// Position of the ship 
	GLfloat direction[COORD] ;				// Direction of translation of the ship 
	GLfloat angle ;						// Angle that ship is pointing
	GLfloat cur_speed ;                         		// Speed of the ship 
	GLfloat acc_speed ;					// Speed component from acceleration 
	GLfloat drag_factor ;					// Drag factor with thrusters off 
	GLint   num_vertices ;                      		// Number of vertices in ship  
	GLint waiting;						// Waiting period of ship
	GLint   alive ;                             		// Has ship been destroyed? 
	GLint   hyperspaced ;					// Flag indiciating if ship is in hyperspace
	GLint   exploding ;					// Flag indicating whether ship is exploding
} ship_type ;

				
typedef struct {						// Data ty[e fpr saucers (bad guys)
	GLfloat saucer[SHIP_VERTEX][COORD];			// vertices of ship
	GLfloat translation[COORD];				// translation of ship
	GLfloat direction[COORD];				// direction of ship
	GLfloat time_limit;		 		    	// time ship should wait before entering
	GLfloat num_vertices;					// number of vertices of ship
	GLfloat shoot_dir[COORD];				// direction of shooting
	GLfloat speed;						// speed of ship
	GLint turn_ctr;						// counter for turning
	GLint last_shot;					// time last shot was fired
	GLint wait_start;					// time when ship started waiting to enter game
	GLint alive;                                		// boolean whether ship is alive
	GLint wait_started;					// boolean whether ship started waiting
	GLint on_screen;					// boolean whether ship is on screen
	GLint last_on_screen;					// time ship was last on screen
	
} bad_guy;

typedef struct {						// Data type for storing specific shape of asteroid
	GLfloat shape[ROCK_VERTEX][COORD] ;
	GLint num_vertices ;
} shape_type ;

typedef struct {						// Data type storing all possible shapes of asteroids
	shape_type shapes[MAX_SHAPES] ;
	GLint last ;
} shapes_type ;

typedef struct {                               			// Data type for an asteroid 
	GLfloat rock[ROCK_VERTEX][COORD] ;
	GLint   num_vertices ;
	GLfloat direction[COORD] ;                        	// Direction of a rock 
	GLfloat translation[COORD] ;                       	// Position of a rock 
	GLfloat speed ;                                       	// Speed of a rock 
	GLint   type ;                                  	// Big, medium, or small 
	GLint   alive ;                               		// Has the rock been shot? 
} rock_type ;

typedef struct {                      				// Data type for a group of asteroids 
	rock_type rocks[MAX_ROCKS] ;
	GLint last ;
} asteroids_type ;

typedef struct { 						// Data type for storing torpedos	
	GLfloat direction[COORD] ;		
	GLfloat translation[COORD] ;
	GLfloat speed ;
	GLint   alive ;
	GLint frames ;						// Array used for storing frames of torpedo (for explosions, bad shots..)
} torpedo_type ;

typedef struct {						// Data type limiting number of simultaneous shots
	torpedo_type torpedoes[MAX_TORPEDOES] ;
} salvo_type ;

typedef struct {						// Data type for animating explosions of ship
	GLfloat direction[NUM_POINTS][COORD] ;
	GLfloat translation[COORD] ;
	GLint num_vertices ;
	GLint frames ;
	GLint alive ;
} debris_type ;

typedef struct {						// Data type storing all explosions of ship
	debris_type debris[MAX_EXPLOSIONS] ;
} explosion_type ;

ship_type       ship ;                                         	// Ship shape 
shapes_type     ast_shapes ;                             	// Asteroids shapes 
asteroids_type  asteroids ;                                     // Asteroids 
salvo_type      salvo ;                                		// Round of torpedoes 
explosion_type  explosion ;                       		// When an asteroid splits 
bad_guy		big_bad;					// Large saucer
bad_guy		small_bad;					// Small saucer

GLboolean  turn_right, turn_left, accelerate, shooting, end_round ;

int num_ships, num_rounds, game_started, game_ended, game_stopped, 
program_started, frame_wait, score, high_score, extra, blink, round_start;

char score_string[SCORE_STR], high_score_string[SCORE_STR] ;

/**
 * \name   itoa
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param n	integer to convert
 * \output param s	array for string
 * Converts integer to string (for storing score string in character array for display)
 */
void itoa(n,s)
int n ;
char s[SCORE_STR] ; 

{ int i, j, digit ;
	char t[SCORE_STR] ;
	//make sure that that score will fit into character array 
	n = n%(MAX_SCORE + 1) ;
	for (i = 0 ; i < SCORE_STR ; i++) {
		s[i] = ' ' ;
	}    
	//if score is zero, display 00 as score
	if (n == 0) {
		s[3] = '0' ;
		s[4] = '0' ;
		s[5] = '\0' ;
	}
	//if score is nonzero
	else {
		i = 0 ;
		//loop through digits and convert int to string
		while (abs(n) > 0) {
			digit = abs(n) % 10 ;
			t[i] = (char)((int)'0' + digit) ;
			n = n / 10 ;
			i++ ;
		}
		t[i] = '\0' ;
		//store score in input paramater
		for (j = 0 ; j < i ; j++) {
			s[SCORE_STR-j-2] = t[j] ;
		}
		s[SCORE_STR - 1] = '\0' ;
	}
}

/**
 * \name   Init
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Launches window for game
 */
void Init()

{ glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB) ;
	glutInitWindowSize(2*(int)MAX_X,2*(int)MAX_Y) ;
	glutInitWindowPosition(0,0) ;
	glutCreateWindow("ASTEROIDS") ;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
	glEnable(GL_BLEND) ;
	glEnable(GL_LINE_SMOOTH) ;
	glShadeModel(GL_FLAT) ;
	glClearColor(0.0, 0.0, 0.0, 0.0) ;
}

/**
 * \name   RandomSpeed
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param type	Size of asteroid
 * \return		Speed of asteroid
 * Method that determines the speed of an asteroid based on its size
 */
float RandomSpeed(type)
int type ;
{ 
	if (type == BIG) {
    		return rand()%(int)BIG_MAX_SPEED + (float)num_rounds/2.0  ;
	}
	else {
    		if (type == MED) {
			return rand()%(int)MED_MAX_SPEED + (float)num_rounds/2.0  ;
    		}
    		else {
			return rand()%(int)SML_MAX_SPEED + (float)num_rounds/2.0  ;
    		}
	}
}

/**
 * \name   RandomDirection
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \output param ux	x-value of vector
 * \output param uy	y-value of vector
 * Calculates random direction for asteroids and explosions
 */
void RandomDirection(ux,uy)
float *ux, *uy ;

{ float n ;
	int s ;
	//assign random x and y translation	
	*ux = rand()%DIRECTION + 1 ;
	*uy = rand()%DIRECTION + 1 ;
	n = sqrt(pow(*ux,2.0)+pow(*uy,2.0)) ;
	*ux /= n ;
	*uy /= n ;
	//randomly determine whether x translation is left or right
	s = rand()%2 ;
	if (s) {
		*ux *= -1.0 ;
	}
	//randomly determine whether y up or down
	s = rand()%2 ;
	if (s) {
		*uy *= -1.0 ;
	}
	
}


/**
 * \name BigShoot
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that controls the shooting of the big saucer.
 */
void BigShoot() {
	//get current time
	int curr_time=time(0);
	//if it has been one second since last shot
	if((curr_time - big_bad.last_shot)>1 ){
		float ux,uy;
		// get a random direction
		RandomDirection(&ux,&uy);
		GLfloat V[COORD], speed ;
		big_bad.shoot_dir[X]=ux;
		big_bad.shoot_dir[Y]=uy;
		int i=0;
		while ((salvo.torpedoes[i].alive) && (i < MAX_TORPEDOES)) {
			i++ ;
		}
		//fire torpedo in random direction
		if (i < MAX_TORPEDOES) {
			salvo.torpedoes[i].alive = TRUE ;
			// set transformations for torpedo
			salvo.torpedoes[i].translation[X] = big_bad.translation[X] + big_bad.shoot_dir[X]*BIG_VERTEX ;
			salvo.torpedoes[i].translation[Y] = big_bad.translation[Y] + big_bad.shoot_dir[Y]*BIG_VERTEX ;   
			V[X] = big_bad.shoot_dir[X]*TORPEDO_SPEED + big_bad.direction[X]*big_bad.speed ;
			V[Y] = big_bad.shoot_dir[Y]*TORPEDO_SPEED + big_bad.direction[Y]*big_bad.speed ;
			speed = sqrt(pow(V[X],2.0) + pow(V[Y],2.0)) ;
			salvo.torpedoes[i].direction[X] = V[X]/speed ;
			salvo.torpedoes[i].direction[Y] = V[Y]/speed ;
			salvo.torpedoes[i].speed = speed ;
			if (salvo.torpedoes[i].speed < TORPEDO_MIN_SPEED) salvo.torpedoes[i].speed = TORPEDO_MIN_SPEED ;
			salvo.torpedoes[i].frames = BAD_SHOT ;
		}
		//store time that last shot was fired
		big_bad.last_shot=time(0);
		
	}
}

/*
 * \name SmallShoot
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that controls the shooting of the small saucers.
 */
void SmallShoot() {
	//get current time
	int curr_time=time(0);
	//if it has been one second since last shot
	if((curr_time - small_bad.last_shot)>1){
		GLfloat ux,uy,mag;
		GLfloat V[COORD], speed ;
		//get vector from small saucer to ships position
		ux=ship.translation[X]-small_bad.translation[X];
		uy=ship.translation[Y]-small_bad.translation[Y];
		//normalize vector
		mag=sqrt( pow(ux,2.0) + pow(uy,2.0));
		ux=ux/mag;
		uy=uy/mag;
		//store vector as shoot direction
		small_bad.shoot_dir[X]=ux;
		small_bad.shoot_dir[Y]=uy;
		int i=0;
		//get number of live torpedoes
		while ((salvo.torpedoes[i].alive) && (i < MAX_TORPEDOES)) {
			i++ ;
		}
		//shoot torpedo in direction of vector
		if (i < MAX_TORPEDOES) {
			//set transformations to torpedo
			salvo.torpedoes[i].alive = TRUE ;
			salvo.torpedoes[i].translation[X] = small_bad.translation[X] + small_bad.shoot_dir[X]*SMALL_VERTEX ;
			salvo.torpedoes[i].translation[Y] = small_bad.translation[Y] + small_bad.shoot_dir[Y]*SMALL_VERTEX ;   
			V[X] = small_bad.shoot_dir[X]*TORPEDO_SPEED ;
			V[Y] = small_bad.shoot_dir[Y]*TORPEDO_SPEED ;
			speed = sqrt(pow(V[X],2.0) + pow(V[Y],2.0)) ;
			salvo.torpedoes[i].direction[X] = V[X]/speed ;
			salvo.torpedoes[i].direction[Y] = V[Y]/speed ;
			salvo.torpedoes[i].speed = speed ;
			if (salvo.torpedoes[i].speed < TORPEDO_MIN_SPEED) salvo.torpedoes[i].speed = TORPEDO_MIN_SPEED ;
			salvo.torpedoes[i].frames = BAD_SHOT ;
		}
		//store time that last shot was fired
		small_bad.last_shot=time(0);
		
	}
}	


/*
 * \name SmallStraight
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing small saucer to go straight
 */
void SmallStraight() {
	small_bad.direction[X]=STRAIGHT_X;
	small_bad.direction[Y]=STRAIGHT_Y;
	//set distance that will continue in this direction
	small_bad.turn_ctr= rand() %(MAX_DISTANCE);
}

/**
 * \name SmallUp
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing small saucer to go up
 */
void SmallUp() {
	small_bad.direction[X]=DIAGONAL;
	small_bad.direction[Y]=DIAGONAL;
	//set distance that will continue in this direction
	small_bad.turn_ctr= rand() % (MAX_DISTANCE);
}

/**
 * \name SmallDown
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing small saucer to go down
 */
void SmallDown() {
	small_bad.direction[X]=DIAGONAL;
	small_bad.direction[Y]=-DIAGONAL;
	//set distance that will continue in this direction
	small_bad.turn_ctr= rand() % (MAX_DISTANCE);
	
}

/**
 * \name BigStraight
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing large saucer to go straight
 */
void BigStraight() {
	big_bad.direction[X]=STRAIGHT_X;
	big_bad.direction[Y]=STRAIGHT_Y;
	//set distance that will continue in this direction
	big_bad.turn_ctr= rand() % (MAX_DISTANCE);
	
}

/**
 * \name BigUp
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing large saucer to go up
 */
void BigUp() {
	big_bad.direction[X]=DIAGONAL;
	big_bad.direction[Y]=DIAGONAL;
	big_bad.turn_ctr= rand() % (MAX_DISTANCE);
	
}

/**
 * \name BigDown
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing large saucer to go down
 */
void BigDown() {
	big_bad.direction[X]=DIAGONAL;
	big_bad.direction[Y]=-DIAGONAL;
	big_bad.turn_ctr= rand() % (MAX_DISTANCE);
	
}

/**
 * \name SmallNewDirection
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing small saucer to get randomly assigned a direction
 * that is is not currently travelling in.
 */
void SmallNewDirection() {
	int randomDir=rand() % 2;
	//if going straight
	if(small_bad.direction[Y]==STRAIGHT_Y){
		if(randomDir)
			SmallUp();
		else 
			SmallDown();
	}
	//if going up
	else if(small_bad.direction[Y]==(DIAGONAL)){
		if(randomDir)
			SmallStraight();
		else 
			SmallDown();
	}
	//if going down
	else {
		if(randomDir)
			SmallStraight();
		else 
			SmallUp();
	}
}

/**
 * \name BigNewDirection
 
 *
 * Method causing large saucer to get randomly assigned a direction
 * that is is not currently travelling in.
 */
void BigNewDirection() {
	int randomDir=rand() % 2;
	//if going straight
	if(big_bad.direction[Y]==STRAIGHT_Y){
		if(randomDir)
			BigUp();
		else 
			BigDown();
	}
	//if going up
	else if(big_bad.direction[Y]==(DIAGONAL)){
		if(randomDir)
			BigStraight();
		else 
			BigDown();
	}
	//if going down
	else {
		if(randomDir)
			BigStraight();
		else 
			BigUp();
	}
	
}

/** 
 * \name   SmallRandomDirection
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing small saucer to get randomly assigned any direction.
 */
void SmallRandomDirection() {
	int randomDir=rand()%3;
	//go straight
	if(randomDir==STRAIGHT) 
		SmallStraight();
	//go up
	else if(randomDir==UP)
		SmallUp();
	//go down
	else
		SmallDown();
}

/**
 * \name BigRandomDirection
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method causing large saucer to get randomly assigned any direction.
 */
void BigRandomDirection() {
	int randomDir=rand()%3;
	//go straight
	if(randomDir==STRAIGHT) 
		BigStraight();
	//go up
	else if(randomDir==UP)
		BigUp();
	//go down
	else
		BigDown();
}


/**
 * \name   RandomPosition
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \output param tx	random x position
 * \output param ty	random y position
 * Assigns random position on screen (for asteroids)
 */
void RandomPosition(tx,ty)
float *tx, *ty ;

{ float ux, uy ;
	int r ;
	RandomDirection(&ux,&uy) ;
	r = rand()%(int)MAX_X  + MAX_X - SAFE_WIDTH ;
	*tx = r*ux ;
	*ty = r*uy ;
}


/**
 * \name   RandomShape
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param num_shapes	number of possible shapes in array
 * \return 			value in range [0,num_shapes)
 * Method for determining shape of asteroids
 */
int RandomShape(num_shapes)
int num_shapes ;

{ 
	return rand()%num_shapes ;
}

/**
 * \name   AsteroidWrapAround
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param i	Index of asteroid in array
 * Method controlling asteroid wrapping around screen
 */
void AsteroidWrapAround(i) 
int i ;

{ 
	//if goes off right, wrap to left
	if (asteroids.rocks[i].translation[X] > MAX_X) {
		asteroids.rocks[i].translation[X] = MIN_X + 1.0 ;
	} 
	else {
		//if goes off left, wrap to right
    		if (asteroids.rocks[i].translation[X] < MIN_X) {
			asteroids.rocks[i].translation[X] = MAX_X - 1.0 ;
    		}
	}
	//if goes off top, wrap to bottom	
	if (asteroids.rocks[i].translation[Y] > MAX_Y) {
		asteroids.rocks[i].translation[Y] = MIN_Y + 1.0 ;
	} 
	else {
		//if goes off bottom, wrap to top
		if (asteroids.rocks[i].translation[Y] < MIN_Y) {
			asteroids.rocks[i].translation[Y] = MAX_Y - 1.0 ; 
		}
	}
}

/**
 * \name   ShipWrapAround
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method controlling ship wrapping around screen
 */
void ShipWrapAround()
{ 
	//if off right, wrap left
	if (ship.translation[X] > MAX_X) {
		ship.translation[X] = MIN_X + 1.0 ;
	} 
	else {
	    //if off left, wrap right
	    if (ship.translation[X] < MIN_X) {
		ship.translation[X] = MAX_X - 1.0 ;
    		}
	}
	//if off top, wrap bottom
	if (ship.translation[Y] > MAX_Y) {
		ship.translation[Y] = MIN_Y + 1.0 ;
	} 
	else {
		//if off bottom, wrap top
		if (ship.translation[Y] < MIN_Y) {
			ship.translation[Y] = MAX_Y - 1.0 ; 
		}
	}
}

/**
 * \name   BigBadGuyWrapAround
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method controlling large saucer wrapping around screen
 */
void BigBadGuyWrapAround()
{
	//get a random y value between the Y_MIN and Y_MAX of viewport
	int max_from_zero=MAX_Y-MIN_Y;
	int y=rand()%max_from_zero + MIN_Y;
	// if saucer to the right of viewport
	if (big_bad.translation[X] > MAX_X) {
		//set saucer to off screen
		if(big_bad.on_screen){
			big_bad.on_screen=FALSE;
			big_bad.last_on_screen=time(0);
		}
		//if time saucer has been off screen is greater than 3 seconds
		else if((time(0)-big_bad.last_on_screen)>3){
			// put saucer back on screen
			big_bad.on_screen=TRUE;
			// wrap saucer to left side of viewport
			big_bad.translation[X] = MIN_X + 1.0 ;
			big_bad.translation[Y]=y +1.0 ;
			BigRandomDirection();
		}
		
	} 
	else {
		//if saucer to the left of viewport
		if (big_bad.translation[X] < MIN_X) {
			//set saucer to off of screen
			if(big_bad.on_screen){
				big_bad.on_screen=FALSE;
				big_bad.last_on_screen=time(0);
			}
			// if time that saucer has been off screen greater than 3 seconds
			else if((time(0) - big_bad.last_on_screen)>3) {
				//put saucer on screen and wrap saucer to left side of viewport
				big_bad.on_screen=TRUE;
				big_bad.translation[X] = MAX_X - 1.0 ;
				big_bad.translation[Y]=y +1.0 ;
				BigRandomDirection();
			}
		}
	}
	//if saucer went off top of view port
	if (big_bad.translation[Y] > MAX_Y) {
		//wrap around to bottom of viewport
		big_bad.translation[Y] = MIN_Y + 1.0 ;
	} 
	else {
		//if saucer went off bottom of viewport
		if (big_bad.translation[Y] < MIN_Y) {
			// wrap saucer to top
			big_bad.translation[Y] = MAX_Y - 1.0 ; 
		}
	}
}

/**
 * \name   SmallBadGuyWrapAround
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method control small saucer wrapping around screen.
 */
void SmallBadGuyWrapAround()

{ 
	//get random y coordinate between top and bottom of viewport
	int max_from_zero=MAX_Y-MIN_Y;
	int y=rand()%max_from_zero + MIN_Y;
	// if saucer went off top of viewport
	if (small_bad.translation[X] > MAX_X) {
		// set saucer to off of viewport
		if(small_bad.on_screen){
			small_bad.on_screen=FALSE;
			small_bad.last_on_screen=time(0);
		}
		// if saucer has been off viewport for over 3 seconds
		else if((time(0)-small_bad.last_on_screen)>3) {
			//bring saucer on viewport
			small_bad.on_screen=TRUE;
			// wrap saucer to left side
			small_bad.translation[X] = MIN_X + 1.0 ;
			small_bad.translation[Y]=y +1.0 ;
			SmallRandomDirection();
		}
	} 
	else {
		//if saucer went off left side of viewport
		if (small_bad.translation[X] < MIN_X) {
			//set saucer to off screen
			if(small_bad.on_screen){
				small_bad.on_screen=FALSE;
				small_bad.last_on_screen=time(0);
			}
			//if saucer been off screen for over 3 seconds
			else if((time(0) - small_bad.last_on_screen)>3) {
				//put saucer on screen
				small_bad.on_screen=TRUE;
				//wrap saucer around screen
				small_bad.translation[X] = MAX_X - 1.0 ;
				small_bad.translation[Y]=y +1.0 ;
				SmallRandomDirection();
			}
		}
	}
	//if saucer went off top of viewport
	if (small_bad.translation[Y] > MAX_Y) {
		//wrap saucer to bottom
		small_bad.translation[Y] = MIN_Y + 1.0 ;
	} 
	else {
		//if saucer went off bottom of viewport
		if (small_bad.translation[Y] < MIN_Y) {
			//wrap saucer to top
			small_bad.translation[Y] = MAX_Y - 1.0 ; 
		}
	}
}

/**
 * \name   TorpedoWrapAround
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method controlling torpedo wrapping around screen
 */
void TorpedoWrapAround()

{ int i ;
	//loop through all live torpedos
	for (i = 0 ; i < MAX_TORPEDOES ; i++) {
		if (salvo.torpedoes[i].alive) {
			//if off right, wrap left
			if (salvo.torpedoes[i].translation[X] > MAX_X) {
				salvo.torpedoes[i].translation[X] = MIN_X + 1.0 ;
			} 
			else {
				//if off left, wrap right
				if (salvo.torpedoes[i].translation[X] < MIN_X) {
					salvo.torpedoes[i].translation[X] = MAX_X - 1.0 ;
				}
			}
			//if off top, wrap bottom
			if (salvo.torpedoes[i].translation[Y] > MAX_Y) {
				salvo.torpedoes[i].translation[Y] = MIN_Y + 1.0 ;
			} 
			else {
				//if off bottom, wrap top
				if (salvo.torpedoes[i].translation[Y] < MIN_Y) {
					salvo.torpedoes[i].translation[Y] = MAX_Y - 1.0 ; 
				}
			}
		}
	}
}

/**
 * \name   GetExplosion
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param f		filename storing explosion information
 * \output param explosion	variable storing explosion data 
 * Read explosion information from data file
 */
int GetExplosion(f,explosion)
char *f ; explosion_type *explosion ;

{ FILE *fd ; 
	float d[COORD] ;
	int num_vertices, i, j ; 
	//if unable to open file display error
	if ((fd = fopen(f,"r")) == NULL) {
		printf("Error: unable to open file containing explosion data (explosion.dat). Exiting application...\n") ;
		exit(0) ;
	} 
	//scan all values into variable
	fscanf(fd,"%d",&num_vertices) ;
	for (i = 0 ; i < num_vertices ; i++) {
		fscanf(fd,"%f",&d[X]) ; 
		fscanf(fd,"%f",&d[Y]) ; 
		for (j = 0 ; j < MAX_EXPLOSIONS ; j++) {
			(*explosion).debris[j].num_vertices = num_vertices ;
			(*explosion).debris[j].direction[i][X] = d[X] ;
			(*explosion).debris[j].direction[i][Y] = d[Y] ;
			(*explosion).debris[j].frames = 0 ;
			(*explosion).debris[j].alive = FALSE ;
		}
	}
	// close file
	close(fd) ;
}

/**
 * \name   GetShip
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param f		filename storing ship information
 * \output param ship		variable storing ship data 
 * Read ship information from data file
 */
int GetShip(f,ship)
char *f ;
ship_type *ship ;
{ FILE *fd ; 
	int i ; 
	//if unable to open file, display error and close program
	if ((fd = fopen(f,"r")) == NULL) {
		printf("Error: unable to open file containing spaceship data (ship.dat). Exiting Application...\n") ;
		exit(0) ;
	} 
	//scan values into variable, and set default values of variable
	fscanf(fd,"%d",&(*ship).num_vertices) ;
	for (i = 0 ; i < (*ship).num_vertices ; i++) {
		fscanf(fd,"%f",&((*ship).ship[i][X])) ; 
		fscanf(fd,"%f",&((*ship).ship[i][Y])) ; 
	}
	(*ship).translation[X] = 0.0 ;
	(*ship).translation[Y] = 0.0 ;
	(*ship).orientation[X] = 0.0 ;
	(*ship).orientation[Y] = 1.0 ;
	(*ship).direction[X]  = 0.0 ;
	(*ship).direction[Y] = 0.0 ;
	(*ship).angle = 0.0 ;
	(*ship).cur_speed = 0.0 ;
	(*ship).acc_speed = 0.0 ;
	(*ship).drag_factor = DRAG_FACTOR ;
	(*ship).alive = TRUE ;
	(*ship).waiting = FALSE ;
	(*ship).exploding = FALSE ;
	//close file
	close(fd) ;
}

/**
 * \name   GetBigBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that gets large saucer using data points in file and
 * stores it in a bad_guy structure.
 *
 * \param f       fileName that has points of large saucer
 * \param saucer  variable to store saucer in
 */
void GetBigBadGuy(f,saucer)
char *f ;
bad_guy *saucer ;

{ FILE *fd ;
	int i ;
	// print error if unable to open file
	if ((fd = fopen(f,"r")) == NULL) {
		printf("Error: unable to open file containing saucer information (saucers.dat). Exiting application...\n") ;
		exit(0) ;
	}
	//throw away non-vertex points
	int throwaway;
	fscanf(fd,"%d",&throwaway);
	fscanf(fd,"%d",&throwaway);
	//scan vertices into bad_guy struct
	for (i = 0 ; i < SAUCER_VERTEX ; i++) {
		fscanf(fd,"%f",&((*saucer).saucer[i][X])) ;
		fscanf(fd,"%f",&((*saucer).saucer[i][Y])) ;
	}
	// store default values in bad_guy data members
	(*saucer).num_vertices=SAUCER_VERTEX;
	(*saucer).translation[X] = 0.0 ;
	(*saucer).translation[Y] = 0.0 ;
	(*saucer).direction[X]  = 0.0 ;
	(*saucer).direction[Y] = 0.0 ;
	// set constant speed
	(*saucer).speed = BAD_SPEED ;
	(*saucer).alive = FALSE ;
	(*saucer).last_shot=time(0);
	(*saucer).wait_started = FALSE;
	(*saucer).time_limit = 0.0;
	(*saucer).wait_start=0.0;
	(*saucer).on_screen=FALSE;
	(*saucer).last_on_screen=0;
	// close file
	close(fd) ;
}

/**
 * \name   GetSmallBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that gets small saucer using data points in file and
 * stores it in a bad_guy structure.
 * \param f       file that has points of small saucer
 * \param saucer  variable to store saucer in
 */
void GetSmallBadGuy(f,saucer)
char *f ;
bad_guy *saucer ;

{ FILE *fd ;
	int i ;
	int foundShip= 0;
	float throwaway=0.0f;
	float currPoint=0.0f;
	//print error if unable to open file
	if ((fd = fopen(f,"r")) == NULL) {
		printf("Error: unable to open file containing saucer data (saucers.dat). Exiting application...\n") ;
		exit(0) ;
	}
	
	//throw away points of large saucer in file
	fscanf(fd,"%f",&throwaway);
	fscanf(fd,"%f",&throwaway) ;
	while(!foundShip){
		fscanf(fd,"%f",&currPoint);
		if((int)currPoint==SAUCER_VERTEX){
			foundShip=1;
		}
	}
	//store vertices of saucer
	for (i = 0 ; i < SAUCER_VERTEX ; i++) {
		fscanf(fd,"%f",&((*saucer).saucer[i][X])) ;
		fscanf(fd,"%f",&((*saucer).saucer[i][Y])) ;
	}
	//set default values of small saucer
	(*saucer).num_vertices=SAUCER_VERTEX;
	(*saucer).translation[X] = 0.0 ;
	(*saucer).translation[Y] = 0.0 ;
	(*saucer).direction[X]  = 0.0 ;
	(*saucer).direction[Y] = 0.0 ;
	(*saucer).speed = BAD_SPEED ;
	(*saucer).alive = FALSE ;
	(*saucer).last_shot=time(0);
	(*saucer).on_screen=FALSE;
	(*saucer).wait_started=FALSE;
	(*saucer).last_on_screen=0;
	//close file
	close(fd) ;
}


/**
 * \name   GetShapes
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param f		filename storing asteroid shape information
 * \output ast_shapes		variable storing shapes of asteroids
 * Read asteroid shape information from data file
 */
int GetShapes(f,ast_shapes)
char *f ;
shapes_type *ast_shapes ;

{ FILE *fd ; 
	int i, j, num_shapes, num_vertex ; 
	//if unable to open file, display error and close
	if ((fd = fopen(f,"rwx")) == NULL) {
		printf("Error: unable to open file containing asteroid data (shapes.dat). Exiting application...\n");
		exit(0) ;
	} 
	//scan shape data into variable and set default values
	fscanf(fd,"%d",&num_shapes) ;
	(*ast_shapes).last = num_shapes ;
	for (i = 0 ; i < num_shapes ; i++) {
		fscanf(fd,"%d",&(*ast_shapes).shapes[i].num_vertices) ;
		for (j = 0 ; j < (*ast_shapes).shapes[i].num_vertices ; j++) {
			fscanf(fd,"%f",&((*ast_shapes).shapes[i].shape[j][X])) ; 
			fscanf(fd,"%f",&((*ast_shapes).shapes[i].shape[j][Y])) ; 
		}
	}
	//close file
	close(fd) ;
}

/**
 * \name   GenerateRound
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param num_asteroids	number of asteroids to generate
 * \input param ast_shapes	variable storing shapes of asteroids
 * \output param asteroids	variable storing asteroids for new round
 * Generate asteroids and saucers for new round
 */
void GenerateRound(ast_shapes,asteroids,num_asteroids)
shapes_type *ast_shapes ;
asteroids_type *asteroids ;
int num_asteroids ;

{ float ux, uy, tx, ty ;
	int shape, i, j ;
	round_start=time(0);
	//set default values of small and big bad guys for new rounds
	big_bad.alive=FALSE;
	big_bad.wait_started=FALSE;
	big_bad.on_screen=FALSE;
	small_bad.on_screen=FALSE;
	small_bad.alive=FALSE;
	small_bad.wait_started=FALSE;
	(*asteroids).last = num_asteroids ;
	//Generate asteroids based on input ast_shapes array
	for (i = 0 ; i < num_asteroids ; i++) {
		//determine shape to base asteroid from
		shape = RandomShape((*ast_shapes).last) ;
		for (j = 0 ; j < (*ast_shapes).shapes[shape].num_vertices ; j++) {
			(*asteroids).rocks[i].rock[j][X] = (*ast_shapes).shapes[shape].shape[j][X] ;
			(*asteroids).rocks[i].rock[j][Y] = (*ast_shapes).shapes[shape].shape[j][Y] ;
		}
		(*asteroids).rocks[i].num_vertices = (*ast_shapes).shapes[shape].num_vertices ;
		(*asteroids).rocks[i].alive = TRUE ;
		(*asteroids).rocks[i].type = BIG ;
		(*asteroids).rocks[i].speed = (GLfloat)RandomSpeed(BIG) ;
		RandomDirection(&ux,&uy) ;
		(*asteroids).rocks[i].direction[X] = ux ;
		(*asteroids).rocks[i].direction[Y] = uy ;
		RandomPosition(&tx,&ty) ;
		(*asteroids).rocks[i].translation[X] = tx ;
		(*asteroids).rocks[i].translation[Y] = ty ;
	}
}

/**
 * \name   GenerateTorpedoes
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \output param salve		variable storing torpedos for new round
 * Refresh default values of torpedo for new round
 */
void GenerateTorpedoes(salvo)
salvo_type *salvo ;

{ int i ;
	//loop through torpedo array and set to default values
	for (i = 0 ; i < MAX_TORPEDOES ; i++) {
		(*salvo).torpedoes[i].direction[X] = 0.0 ;
		(*salvo).torpedoes[i].direction[Y] = 1.0 ;
		(*salvo).torpedoes[i].translation[X] = 0.0 ;
		(*salvo).torpedoes[i].translation[Y] = 0.0 ;
		(*salvo).torpedoes[i].frames = 0 ;
		(*salvo).torpedoes[i].alive = FALSE ;
	}
}

/**
 * \name   TextOutput
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param s		score
 * \input param x		x position of score string on window
 * \input param y		y position of score string on window
 * \output param text		variable storing score string
 * Convert integer score to string and display
 */
void TextOutput(s,x,y,text)
GLfloat s, x, y ;
char *text ;

{ char *p ;
	
	glPushMatrix() ;
	glTranslatef(x,y,0.0) ;
	glScalef(s,s,1.0) ;
	for (p = text ; *p ; p++) {
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,*p) ;
	}
	glPopMatrix() ;
}
void SetShipNormals(ship)
ship_type *ship ;

{ int i ;
	
	for (i = 0 ; i < (*ship).num_vertices-1 ; i++) {
		RandomDirection(&((*ship).normal[i][X]),(&(*ship).normal[i][Y])) ;
		(*ship).frames[i] = rand()%(WRECK_WAIT+1) ;
	}
}
/**
 * \name   Hyperspace
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Implements hyperspace functionality (randomly generates new position for ship)
 */
void HyperSpace()

{ int loss, i ;
	
	if (ship.alive) {
		ship.hyperspaced = TRUE ;
		ship.translation[X] = (float)(rand()%(int)MAX_X*2 + MIN_X)  ;
		ship.translation[Y] = (float)(rand()%(int)MAX_Y*2 + MIN_Y)  ;
		frame_wait = HYPER_WAIT ;
	}
}

/**
 * \name   ReEntry
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method controlling reentry after hypersapce (1/5 change of explosion)
 */
void ReEntry() 

{ int loss ;
	
	loss = rand()%EXP_ON_REENTRY ;
	//if ship exploded	
	if (loss == EXP_ON_REENTRY - 1) {
		ship.alive = FALSE ;
		ship.exploding = TRUE ;
		frame_wait = WRECK_WAIT ;
		SetShipNormals(&ship) ;
		num_ships-- ;
	}
	else {
		ship.cur_speed = 0.0 ;
	}
}

/**
 * \name   SuitableTime
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method ensuring that ship enters game when there are no asteroids or saucers in the center of the window
 */
int SuitableTime()

{ int i ; float a ;
	
	for (i = 0 ; i < asteroids.last ; i++) {
		if (asteroids.rocks[i].alive) {
			if (sqrt(pow(asteroids.rocks[i].translation[X],2.0) 
					 + pow(asteroids.rocks[i].translation[Y],2.0)) <= SAFE_LIMIT) {
				return FALSE ;
			}
		}
		//if big saucer is on screen
		if(big_bad.alive && big_bad.on_screen){
			//if big saucer is not in safe distance from center of viewport return false
			if(sqrt(pow(big_bad.translation[X],2.0)+pow(big_bad.translation[Y],2.0)) <= SAFE_LIMIT) {
				return FALSE;
			}
		}
		//if small saucer is on screen
		if(small_bad.alive && small_bad.on_screen) {
			//if small saucer is not in safe distance from center of viewport return false
			if(sqrt(pow(small_bad.translation[X],2.0)+pow(small_bad.translation[Y],2.0))<=SAFE_LIMIT){
				return FALSE;
			}
		}
	}
	return TRUE ;
}

/**
 * \name   SplitRock
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param	index of rock to be split
 * Method control splitting of asteroids (on intersection with ship, saucer, or torpedo)
 */
void SplitRock(i)
int i ;

{ float divisor, ux, uy, tx, ty ;
	int shape, j, k ;
	
	if (asteroids.rocks[i].type != SML) {
		divisor = (float)(asteroids.rocks[i].type)*2.0 ;
		for (k = asteroids.last ; k < asteroids.last + 2 ; k++) {
			asteroids.rocks[k].translation[X] = asteroids.rocks[i].translation[X] ;
			asteroids.rocks[k].translation[Y] = asteroids.rocks[i].translation[Y] ;
			
			shape = RandomShape(ast_shapes.last) ;
			for (j = 0 ; j < ast_shapes.shapes[shape].num_vertices ; j++) {
				asteroids.rocks[k].rock[j][X] = ast_shapes.shapes[shape].shape[j][X]/divisor ;
				asteroids.rocks[k].rock[j][Y] = ast_shapes.shapes[shape].shape[j][Y]/divisor ;
			}
			asteroids.rocks[k].num_vertices = ast_shapes.shapes[shape].num_vertices ;
			
			asteroids.rocks[k].alive = TRUE ;
			asteroids.rocks[k].type = asteroids.rocks[i].type + 1 ;
			asteroids.rocks[k].speed = (GLfloat)RandomSpeed(asteroids.rocks[k].type) ;
			RandomDirection(&ux,&uy) ;
			asteroids.rocks[k].direction[X] = ux ;
			asteroids.rocks[k].direction[Y] = uy ;
		}
		asteroids.last += 2 ;
	}
}

/**
 * \name   BigBadGuyAsteroids
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and deals with the case when big saucers collide with
 * asteroids.
 * /param i index of vertex of large saucer
 * /param j index of asteroid
 */
void BigBadGuyAsteroids(i,j)
int i, j ;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], p1[COORD], p2[COORD],  det, u, v ;
	int intersect, k, l ;
	//store points of line in saucer
	p1[X] = big_bad.saucer[i][X] ;
	p1[Y] = big_bad.saucer[i][Y] ;
	p2[X] = big_bad.saucer[i+1][X] ;
	p2[Y] = big_bad.saucer[i+1][Y] ;
	//translate points of saucer
	s1[X] = p1[X] + big_bad.translation[X] ;
	s1[Y] = p1[Y] + big_bad.translation[Y] ;
	s2[X] = p2[X] + big_bad.translation[X] ;
	s2[Y] = p2[Y] + big_bad.translation[Y] ;
	
	k = 0 ;
	intersect = FALSE ;
	//loop through line segments of asteroid and check for intersection
	while (( k < asteroids.rocks[j].num_vertices - 1) && (!intersect)) {
		//get translated line segment of asteroid
		t1[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k][X] ;
		t1[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k][Y] ;
		t2[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k+1][X] ;
		t2[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k+1][Y] ;
		//check for intersection of line segments
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			//if line segments intersect
			if ((u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0)) {
				//exit loop
				intersect = TRUE ;
				// asteroid and saucer set to dead
				asteroids.rocks[j].alive = FALSE ;
				big_bad.alive = FALSE ;
				big_bad.last_shot=time(0);
				big_bad.on_screen=FALSE;
				big_bad.wait_started=FALSE;
				big_bad.last_on_screen=0;
				
				//split rock that intersected with big saucer
				SplitRock(j) ;
				l = 0 ;
				//find index of explosion debris that is not holding live debris
				while ((explosion.debris[l].alive) && (l < MAX_EXPLOSIONS)) {
					l++ ;
				}
				//trigger explosion debris at location of asteroid
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = asteroids.rocks[j].translation[X] ;
					explosion.debris[l].translation[Y] = asteroids.rocks[j].translation[Y] ;
				}
				l++;
				//trigger explosion debris at location of saucer
				if (l < MAX_EXPLOSIONS ) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = big_bad.translation[X] ;
					explosion.debris[l].translation[Y] = big_bad.translation[Y] ;
				}
				
			}
		}
		//increment index to next line segment of asteroid
		k++ ;
	}
}

/**
 * \name   SmallBadGuyAsteroids
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and deals with the case when small saucers collide with
 * asteroids.
 * /param i index of vertex of small saucer
 * /param j index of asteroid
 */
void SmallBadGuyAsteroids(i,j)
int i, j ;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], p1[COORD], p2[COORD], det, u, v ;
	int intersect, k, l ;
	// Store points of line segment of small saucer
	p1[X] = small_bad.saucer[i][X] ;
	p1[Y] = small_bad.saucer[i][Y] ;
	p2[X] = small_bad.saucer[i+1][X] ;
	p2[Y] = small_bad.saucer[i+1][Y] ;
	
	// Translate points of line segment of small saucer
	s1[X] = p1[X] + small_bad.translation[X] ;
	s1[Y] = p1[Y] + small_bad.translation[Y] ;
	s2[X] = p2[X] + small_bad.translation[X] ;
	s2[Y] = p2[Y] + small_bad.translation[Y] ;
	
	k = 0 ;
	intersect = FALSE ;
	
	// Loop through line segments of asteroid and check for intersection
	while (( k < asteroids.rocks[j].num_vertices - 1) && (!intersect)) {
		// Store translated points points of line segments
		t1[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k][X] ;
		t1[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k][Y] ;
		t2[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k+1][X] ;
		t2[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k+1][Y] ;
		
		// Check for intersections
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			//If line segments intersect
			if ((u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0)) {
				intersect = TRUE ;
				
				//set asteroid and small saucer to default values
				asteroids.rocks[j].alive = FALSE ;
				small_bad.alive = FALSE ;
				small_bad.last_shot=time(0);
				small_bad.on_screen=FALSE;
				small_bad.wait_started=FALSE;
				small_bad.last_on_screen=0;
				
				//split asteroid
				SplitRock(j) ;
				l = 0 ;
				
				//count number of live explosions
				while ((explosion.debris[l].alive) && (l < MAX_EXPLOSIONS)) {
					l++ ;
				}
				
				//trigger explosion at location of asteroid
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = asteroids.rocks[j].translation[X] ;
					explosion.debris[l].translation[Y] = asteroids.rocks[j].translation[Y] ;
				}
				l++;
				//trigger explosion debris at location of saucer
				if (l < MAX_EXPLOSIONS ) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = small_bad.translation[X] ;
					explosion.debris[l].translation[Y] = small_bad.translation[Y] ;
				}
			}
		}
		//increment index to next line segment of asteroid
		k++ ;
	}
}

/**
 * \name   ShipBigBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and deals with intersections between the ship and the big saucer
 * \param i index of vertex of ship
 */
void ShipBigBadGuy(i)
int i;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], p1[COORD], p2[COORD], r[COORD], det, u, v ;
	int intersect, k, l ;
	// Store points of line segment of ship
	p1[X] = ship.ship[i][X] ;
	p1[Y] = ship.ship[i][Y] ;
	p2[X] = ship.ship[i+1][X] ;
	p2[Y] = ship.ship[i+1][Y] ;
	
	//Rotate points
	r[X] = p1[X]*cos(-ship.angle*DEG_RAD_CONV) + p1[Y]*sin(-ship.angle*DEG_RAD_CONV) ; 
	r[Y] = p1[Y]*cos(-ship.angle*DEG_RAD_CONV) - p1[X]*sin(-ship.angle*DEG_RAD_CONV) ; 
	
	p1[X] = r[X] ;
	p1[Y] = r[Y] ;
	
	r[X] = p2[X]*cos(-ship.angle*DEG_RAD_CONV) + p2[Y]*sin(-ship.angle*DEG_RAD_CONV) ; 
	r[Y] = p2[Y]*cos(-ship.angle*DEG_RAD_CONV) - p2[X]*sin(-ship.angle*DEG_RAD_CONV) ; 
	
	p2[X] = r[X] ;
	p2[Y] = r[Y] ;
	
	// Tranlate points of line segment of ship
	s1[X] = p1[X] + ship.translation[X] ;
	s1[Y] = p1[Y] + ship.translation[Y] ;
	s2[X] = p2[X] + ship.translation[X] ;
	s2[Y] = p2[Y] + ship.translation[Y] ;
	
	k = 0 ;
	intersect = FALSE ;
	// Loop through line segments of big saucer and check for intersection
	while (( k < big_bad.num_vertices - 1) && (!intersect)) {
		// Store translated points of line segment of big saucer
		t1[X] = big_bad.translation[X] + big_bad.saucer[k][X] ;
		t1[Y] = big_bad.translation[Y] + big_bad.saucer[k][Y] ;
		t2[X] = big_bad.translation[X] + big_bad.saucer[k+1][X] ;
		t2[Y] = big_bad.translation[Y] + big_bad.saucer[k+1][Y] ;
		
		// Check for intersection
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			// If line segments intersect
			if ((u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0)) {
				intersect = TRUE ;
				
				//inc score
				score+=200;
				extra+=200;
				itoa(score,score_string) ;
				if (extra - EXTRA_SHIP >= 0) { 
					num_ships++ ;
					extra -= EXTRA_SHIP ;
				}
				
				//set large saucer to default values
				big_bad.alive = FALSE ;
				big_bad.last_shot=time(0);
				big_bad.on_screen=FALSE;
				big_bad.wait_started=FALSE;
				big_bad.last_on_screen=0;
				
				// Set ship to dead and exploding
				ship.alive = FALSE ;
				ship.exploding = TRUE ;
				
				// Subtract one ship from game
				frame_wait = WRECK_WAIT ;
				SetShipNormals(&ship) ;
				num_ships-- ;
				
				l = 0 ;
				// Count number of live explosions
				while ((explosion.debris[l].alive) && (l < MAX_EXPLOSIONS)) {
					l++ ;
				}
				
				//Trigger explosion at location of large saucer
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = big_bad.translation[X] ;
					explosion.debris[l].translation[Y] = big_bad.translation[Y] ;
				}
			}
		}
		//increment index to next line segment of big saucer
		k++ ;
	}
}

/**
 * \name   ShipSmallBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and deals with intersections between the ship and the small saucer
 * \param i index of vertex of ship
 */
void ShipSmallBadGuy(i)
int i;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], p1[COORD], p2[COORD], r[COORD], det, u, v ;
	int intersect, k, l ;
	
	//store points of line segment
	p1[X] = ship.ship[i][X] ;
	p1[Y] = ship.ship[i][Y] ;
	p2[X] = ship.ship[i+1][X] ;
	p2[Y] = ship.ship[i+1][Y] ;
	
	// Rotate points of line segment
	r[X] = p1[X]*cos(-ship.angle*DEG_RAD_CONV) + p1[Y]*sin(-ship.angle*DEG_RAD_CONV) ; 
	r[Y] = p1[Y]*cos(-ship.angle*DEG_RAD_CONV) - p1[X]*sin(-ship.angle*DEG_RAD_CONV) ; 
	
	p1[X] = r[X] ;
	p1[Y] = r[Y] ;
	
	r[X] = p2[X]*cos(-ship.angle*DEG_RAD_CONV) + p2[Y]*sin(-ship.angle*DEG_RAD_CONV) ; 
	r[Y] = p2[Y]*cos(-ship.angle*DEG_RAD_CONV) - p2[X]*sin(-ship.angle*DEG_RAD_CONV) ; 
	
	p2[X] = r[X] ;
	p2[Y] = r[Y] ;
	
	// Translate points of line segment
	s1[X] = p1[X] + ship.translation[X] ;
	s1[Y] = p1[Y] + ship.translation[Y] ;
	s2[X] = p2[X] + ship.translation[X] ;
	s2[Y] = p2[Y] + ship.translation[Y] ;
	
	k = 0 ;
	intersect = FALSE ;
	//Loop through line segments of small saucer and check for intersection
	while (( k < small_bad.num_vertices - 1) && (!intersect)) {
		// Store the translated points of a line segment of the small saucer
		t1[X] = small_bad.translation[X] + small_bad.saucer[k][X] ;
		t1[Y] = small_bad.translation[Y] +	small_bad.saucer[k][Y] ;
		t2[X] = small_bad.translation[X] + small_bad.saucer[k+1][X] ;
		t2[Y] = small_bad.translation[Y] + small_bad.saucer[k+1][Y] ;
		
		// Check for intersection between line segments
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			// If line segments intersect
			if ((u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0)) {
				intersect = TRUE ;
				// inc score
				score+=1000;
				extra+=1000;
				itoa(score,score_string) ;
				if (extra - EXTRA_SHIP >= 0) { 
					num_ships++ ;
					extra -= EXTRA_SHIP ;
				}
				// set small saucer to default values
				small_bad.alive = FALSE ;
				small_bad.last_shot=time(0);
				small_bad.on_screen=FALSE;
				small_bad.wait_started=FALSE;
				small_bad.last_on_screen=0;
				// Set ship to dead and exploding
				ship.alive = FALSE ;
				ship.exploding = TRUE ;
				
				// Subtract ship from game
				frame_wait = WRECK_WAIT ;
				SetShipNormals(&ship) ;
				num_ships-- ;
				
				l = 0 ;
				// Count number of live explosions
				while ((explosion.debris[l].alive) && (l < MAX_EXPLOSIONS)) {
					l++ ;
				}
				// Trigger explosion at location of small saucer
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = small_bad.translation[X] ;
					explosion.debris[l].translation[Y] = small_bad.translation[Y] ;
				}
			}
		}
		k++ ;
	}
}
/**
 * \name   ShipAsteroids
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and deals with the case when the ship collide with
 * asteroids.
 * /param i index of vertex of ship
 * /param j index of asteroid
 */
void ShipAsteroids(i,j)
int i, j ;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], p1[COORD], p2[COORD], r[COORD], det, u, v ;
	int intersect, k, l ;
	
	p1[X] = ship.ship[i][X] ;
	p1[Y] = ship.ship[i][Y] ;
	p2[X] = ship.ship[i+1][X] ;
	p2[Y] = ship.ship[i+1][Y] ;
	
	r[X] = p1[X]*cos(-ship.angle*DEG_RAD_CONV) + p1[Y]*sin(-ship.angle*DEG_RAD_CONV) ; 
	r[Y] = p1[Y]*cos(-ship.angle*DEG_RAD_CONV) - p1[X]*sin(-ship.angle*DEG_RAD_CONV) ; 
	
	p1[X] = r[X] ;
	p1[Y] = r[Y] ;
	
	r[X] = p2[X]*cos(-ship.angle*DEG_RAD_CONV) + p2[Y]*sin(-ship.angle*DEG_RAD_CONV) ; 
	r[Y] = p2[Y]*cos(-ship.angle*DEG_RAD_CONV) - p2[X]*sin(-ship.angle*DEG_RAD_CONV) ; 
	
	p2[X] = r[X] ;
	p2[Y] = r[Y] ;
	
	s1[X] = p1[X] + ship.translation[X] ;
	s1[Y] = p1[Y] + ship.translation[Y] ;
	s2[X] = p2[X] + ship.translation[X] ;
	s2[Y] = p2[Y] + ship.translation[Y] ;
	
	k = 0 ;
	intersect = FALSE ;
	while (( k < asteroids.rocks[j].num_vertices - 1) && (!intersect)) {
		t1[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k][X] ;
		t1[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k][Y] ;
		t2[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k+1][X] ;
		t2[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k+1][Y] ;
		
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			if ((u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0)) {
				intersect = TRUE ;
				switch(asteroids.rocks[j].type) {
					case BIG: score += 20 ;
						extra += 20 ;
						break ;
					case MED: score += 50 ;
						extra += 50 ;
						break ;
					case SML: score += 100 ;
						extra += 100 ;
						break ;
					default:  break ;
				}
				itoa(score,score_string) ;
				if (extra - EXTRA_SHIP >= 0) { 
					num_ships++ ;
					extra -= EXTRA_SHIP ;
				}
				asteroids.rocks[j].alive = FALSE ;
				ship.alive = FALSE ;
				ship.exploding = TRUE ;
				frame_wait = WRECK_WAIT ;
				SetShipNormals(&ship) ;
				num_ships-- ;
				SplitRock(j) ;
				l = 0 ;
				while ((explosion.debris[l].alive) && (l < MAX_EXPLOSIONS)) {
					l++ ;
				}
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = asteroids.rocks[j].translation[X] ;
					explosion.debris[l].translation[Y] = asteroids.rocks[j].translation[Y] ;
				}
			}
		}
		k++ ;
	}
}



/**
 * \name   TorpedoShip
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and takes care of the case when a torpedo intersects with the ship
 * \param i index of torpedo
 */
void TorpedoShip(i) 
int i;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], det, u, v ;
	int k, l, intersect ;
	
	// Store points of segment of path of torpedo
	s1[X] = salvo.torpedoes[i].translation[X] ;
	s1[Y] = salvo.torpedoes[i].translation[Y] ;
	s2[X] = salvo.torpedoes[i].translation[X] + salvo.torpedoes[i].direction[X]*salvo.torpedoes[i].speed ;
	s2[Y] = salvo.torpedoes[i].translation[Y] + salvo.torpedoes[i].direction[Y]*salvo.torpedoes[i].speed ;
	
	k = 0 ;
	intersect = FALSE ;
	// Loop through line segments of ship and check for intersection
	while (( k < ship.num_vertices - 1) && (!intersect)) {
		// Store translated points of ship
		t1[X] = ship.translation[X] + ship.ship[k][X] ;
		t1[Y] = ship.translation[Y] + ship.ship[k][Y] ;
		t2[X] = ship.translation[X] + ship.ship[k+1][X] ;
		t2[Y] = ship.translation[Y] + ship.ship[k+1][Y] ;
		
		// Check for intersection of line segments
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			// If line segments intersect
			if ((u >= -0.2) && (u <= 1.2) && (v >= -0.2) && (v <= 1.2)) {
				intersect = TRUE ;
				// Set torpedo and ship to dead
				salvo.torpedoes[i].alive = FALSE ;
				ship.alive = FALSE ;
				// Set ship to exploding and subtracting ship from game
				ship.exploding = TRUE ;
				frame_wait = WRECK_WAIT ;
				SetShipNormals(&ship) ;
				num_ships-- ;
			}
		}
		// Increment to next line segment of ship
		k++ ;
	}
}

/**
 * \name   TorpedoSmallBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and takes care of the case when a torpedo intersects with the small saucer 
 * \param i index of torpedo
 */
void TorpedoSmallBadGuy(i) 
int i;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], det, u, v ;
	int k, l, intersect ;
	
	//Store points of segment of torpedos path
	s1[X] = salvo.torpedoes[i].translation[X] ;
	s1[Y] = salvo.torpedoes[i].translation[Y] ;
	s2[X] = salvo.torpedoes[i].translation[X] + salvo.torpedoes[i].direction[X]*salvo.torpedoes[i].speed ;
	s2[Y] = salvo.torpedoes[i].translation[Y] + salvo.torpedoes[i].direction[Y]*salvo.torpedoes[i].speed ;
	
	k = 0 ;
	intersect = FALSE ;
	// Loop through segments of small saucer and check for intersection
	while (( k < small_bad.num_vertices - 1) && (!intersect)) {
		// Store translated points of small saucer
		t1[X] = small_bad.translation[X] + small_bad.saucer[k][X] ;
		t1[Y] = small_bad.translation[Y] +	small_bad.saucer[k][Y] ;
		t2[X] = small_bad.translation[X] + small_bad.saucer[k+1][X] ;
		t2[Y] = small_bad.translation[Y] + small_bad.saucer[k+1][Y] ;
		
		// Check for intersection of line segments
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			// If segments intersect
			if ((u >= -0.2) && (u <= 1.2) && (v >= -0.2) && (v <= 1.2)) {
				intersect = TRUE ;
				// inc score
				score += 1000 ;
				extra += 1000 ;
				// give extra ship if reached 10,000pts
				itoa(score,score_string) ;
				if (extra - EXTRA_SHIP >= 0) { 
					num_ships++ ;
					extra -= EXTRA_SHIP ;
				}
				// set small saucer to default values
				small_bad.alive = FALSE ;
				small_bad.last_shot=time(0);
				small_bad.on_screen=FALSE;
				small_bad.wait_started=FALSE;
				small_bad.last_on_screen=0;
				salvo.torpedoes[i].alive = FALSE ;
				
				// Count number of live explosions
				l = 0 ;
				while ((explosion.debris[l].alive) && (i < MAX_EXPLOSIONS)) {
					l++ ;
				}
				//trigger location at location of small saucer
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = small_bad.translation[X] ;
					explosion.debris[l].translation[Y] = small_bad.translation[Y] ;
				}
			}
		}
		// increment to next line segment of small saucer
		k++ ;
	}
}

/*
 * \name   TorpedoBigBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks for, and takes care of the case when a torpedo intersects with a big saucer
 * \param i index of torpedo
 */
void TorpedoBigBadGuy(i) 
int i;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], det, u, v ;
	int k, l, intersect ;
	// store points of segment of torpedos path
	s1[X] = salvo.torpedoes[i].translation[X] ;
	s1[Y] = salvo.torpedoes[i].translation[Y] ;
	s2[X] = salvo.torpedoes[i].translation[X] + salvo.torpedoes[i].direction[X]*salvo.torpedoes[i].speed ;
	s2[Y] = salvo.torpedoes[i].translation[Y] + salvo.torpedoes[i].direction[Y]*salvo.torpedoes[i].speed ;
	
	k = 0 ;
	intersect = FALSE ;
	// loop through line segments of big saucer and check for intersection
	while (( k < big_bad.num_vertices - 1) && (!intersect)) {
		// store translated points of line segment of big saucer
		t1[X] = big_bad.translation[X] + big_bad.saucer[k][X] ;
		t1[Y] = big_bad.translation[Y] + big_bad.saucer[k][Y] ;
		t2[X] = big_bad.translation[X] + big_bad.saucer[k+1][X] ;
		t2[Y] = big_bad.translation[Y] + big_bad.saucer[k+1][Y] ;
		
		// check for intersection of line segments
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			// if segments intersect
			if ((u >= -0.2) && (u <= 1.2) && (v >= -0.2) && (v <= 1.2)) {
				intersect = TRUE ;
				//inc score 
				score += 200 ;
				extra += 200 ;
				// if score reached 10000pts inc number of ships
				itoa(score,score_string) ;
				if (extra - EXTRA_SHIP >= 0) { 
					num_ships++ ;
					extra -= EXTRA_SHIP ;
				}
				// set large saucer to default values
				big_bad.alive = FALSE ;
				big_bad.last_shot=time(0);
				big_bad.on_screen=FALSE;
				big_bad.wait_started=FALSE;
				big_bad.last_on_screen=0;
				salvo.torpedoes[i].alive = FALSE ;
				
				// Count number of live explosions
				l = 0 ;
				while ((explosion.debris[l].alive) && (i < MAX_EXPLOSIONS)) {
					l++ ;
				}
				// Trigger explosion at location of large saucer
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = big_bad.translation[X] ;
					explosion.debris[l].translation[Y] = big_bad.translation[Y] ;
				}
			}
		}
		k++ ;
	}
}
/**
 * \name   TorpedoAsteroids
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * /param i index of torpedo in salvo array
 * /param j index of asteroid
 * Method that checks for, and deals with the case when torpedos collide with asteroids.
 */
void TorpedoAsteroids(i,j) 
int i, j ;

{ float s1[COORD], s2[COORD], t1[COORD], t2[COORD], det, u, v ;
	int k, l, intersect ;
	
	s1[X] = salvo.torpedoes[i].translation[X] ;
	s1[Y] = salvo.torpedoes[i].translation[Y] ;
	s2[X] = salvo.torpedoes[i].translation[X] + salvo.torpedoes[i].direction[X]*salvo.torpedoes[i].speed ;
	s2[Y] = salvo.torpedoes[i].translation[Y] + salvo.torpedoes[i].direction[Y]*salvo.torpedoes[i].speed ;
	
	k = 0 ;
	intersect = FALSE ;
	while (( k < asteroids.rocks[j].num_vertices - 1) && (!intersect)) {
		t1[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k][X] ;
		t1[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k][Y] ;
		t2[X] = asteroids.rocks[j].translation[X] + asteroids.rocks[j].rock[k+1][X] ;
		t2[Y] = asteroids.rocks[j].translation[Y] + asteroids.rocks[j].rock[k+1][Y] ;
		
		det = (s2[X] - s1[X])*(t1[Y] - t2[Y]) - (t1[X] - t2[X])*(s2[Y] - s1[Y]) ;
		
		if (fabs(det) >= SMALL) {
			u = 1.0/det*((t1[Y] - t2[Y])*(t1[X] - s1[X]) - (t1[X] - t2[X])*(t1[Y] - s1[Y])) ;
			v = 1.0/det*((s2[X] - s1[X])*(t1[Y] - s1[Y]) - (s2[Y] - s1[Y])*(t1[X] - s1[X])) ;
			
			if ((u >= -0.2) && (u <= 1.2) && (v >= -0.2) && (v <= 1.2)) {
				intersect = TRUE ;
				switch(asteroids.rocks[j].type) {
					case BIG: score += 20 ;
						extra += 20 ;
						break ;
					case MED: score += 50 ;
						extra += 50 ;
						break ;
					case SML: score += 100 ;
						extra += 100 ;
						break ;
					default:  break ;
				}
				itoa(score,score_string) ;
				if (extra - EXTRA_SHIP >= 0) { 
					num_ships++ ;
					extra -= EXTRA_SHIP ;
				}
				asteroids.rocks[j].alive = FALSE ;
				salvo.torpedoes[i].alive = FALSE ;
				SplitRock(j) ;
				l = 0 ;
				while ((explosion.debris[l].alive) && (i < MAX_EXPLOSIONS)) {
					l++ ;
				}
				if (l < MAX_EXPLOSIONS) {
					explosion.debris[l].alive = TRUE ;
					explosion.debris[l].frames = EXPLOSION_FRAMES ;
					explosion.debris[l].translation[X] = asteroids.rocks[j].translation[X] ;
					explosion.debris[l].translation[Y] = asteroids.rocks[j].translation[Y] ;
				}
			}
		}
		k++ ;
	}
}

/**
 * \name   CheckSmallBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks if small saucer should be sent into game.
 */
void CheckSmallBadGuy() {
	//store distance of score from 10000pts
	int highScore=abs(score-EXTRA_SHIP);
	float waitTime=SMALL_WAIT;
	float base;
	int numAsteroids=0,i;
	
	//count number of big asteroids in the game
	for(i=0;i<asteroids.last;i++) {
		if(asteroids.rocks[i].type==BIG && asteroids.rocks[i].alive){
			numAsteroids++;
		}
	}
	
	// Store amount of time from when the round began to current time
	int roundTime=time(0)-round_start;
	
	//If ship should be sent in soon
	if(ship.alive && (!small_bad.alive) && (numAsteroids<=MAX_AST)  &&roundTime>MIN_WAIT){
		
		// If ship is waiting to be sent into game
		if(small_bad.wait_started ){
			
			// If ship has waited enough time to be sent into game
			if((time(0)-small_bad.wait_start)>small_bad.time_limit){
				//send small saucer into game
				small_bad.alive=TRUE;
				small_bad.on_screen=TRUE;
				small_bad.last_shot=time(0);
				small_bad.wait_started = FALSE;
				
				// randomly send in saucer on left or right side of screen
				if(rand()%2)
					small_bad.translation[X] = MIN_X +1.0;
				else{
					small_bad.translation[X]= MAX_X - 1.0;
					SmallRandomDirection();
					small_bad.direction[X]*=-1;
				}
				// Set y value of saucer to random value within screen
				float tx, ty;
				RandomPosition(&tx,&ty);
				small_bad.translation[Y] = ty ;
			}
		}
		
		// Else, set saucer to start waiting to be sent into game
		else {
			small_bad.wait_started=TRUE;
			
			//Calculate amount of time saucer should wait to be sent into game
			if(highScore<MORE_BAD_GUYS)
				base=HIGH_DECREASE;
			else 
				base=NORMAL_DECREASE;
			small_bad.time_limit=waitTime*pow(base,num_rounds);
			
			//Store time when small saucer started waiting
			small_bad.wait_start=time(0);
		}
	}
}

/**
 * \name   CheckBigBadGuy
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that checks if large saucer should be sent into game
 */
void CheckBigBadGuy() {
	// Store distance of score from 10,000pts
	int highScore=abs(score-EXTRA_SHIP);
	float waitTime=LARGE_WAIT;
	float base;
	int numAsteroids=0,i;
	//Count number of big asteroids in game
	for(i=0;i<asteroids.last;i++) {
		if(asteroids.rocks[i].type==BIG && asteroids.rocks[i].alive){
			numAsteroids++;
			
		}
	}
	int roundTime=time(0)-round_start;
	
	//If ship should be sent into the game soon
	if(ship.alive && (!big_bad.alive) && (numAsteroids<=MAX_AST) &&roundTime>MIN_WAIT){
		// If ship has already started waiting to be sent into game
		if(big_bad.wait_started ){
			//if ship has waited enough time to be sent into game
			if((time(0)-big_bad.wait_start)>big_bad.time_limit){
				//send big saucer into game
				big_bad.alive=TRUE;
				big_bad.on_screen=TRUE;
				big_bad.last_shot=time(0);
				big_bad.wait_started = FALSE;
				//randomly put saucer on left or right side of screen
				if(rand()%2)
					big_bad.translation[X] = MIN_X +1.0;
				else{
					big_bad.translation[X]= MAX_X - 1.0;
					BigRandomDirection();
					big_bad.direction[X]*=-1;
				}
				//assign random value to y translation 
				float tx, ty;
				RandomPosition(&tx,&ty);
				big_bad.translation[Y] = ty ;
			}
		}
		// Else, set saucer to start waiting to be sent into game
		else {
			big_bad.wait_started=TRUE;
			// Calculate amount of time saucer should wait before being sent into game
			if(highScore<MORE_BAD_GUYS)
				base=HIGH_DECREASE;
			else 
				base=NORMAL_DECREASE;
			big_bad.time_limit=waitTime*pow(base,num_rounds);
			big_bad.wait_start=time(0);
		}
	}
}
/*
* \name   RenderFrame
* \author Manor Freeman 
* \date   2012/10/26
*
* Method that specifies what to display on screen
*/
void RenderFrame()

{ int i, j ;
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	glColor3f(1.0,1.0,1.0) ;
	glLineWidth(1.8) ;
	TextOutput(0.10,-40.0,280.0,high_score_string) ;
	glColor3f(1.0,1.0,1.0) ;
	glLineWidth(2.8) ;
	TextOutput(0.16,-399.0,270.0,score_string) ;
	
	glColor3f(0.7,0.7,0.7) ;
	glLineWidth(1.0) ;
	TextOutput(0.08,-50.0,-280.0,"(c) ATARI INC") ;
	
	if ((game_ended) && (program_started)) {
		glColor3f(1.0,1.0,1.0) ;
		glLineWidth(2.0) ;
		TextOutput(0.50,-240.0,150.0,"ASTEROIDS") ;
		TextOutput(0.16,-190.0,50.0,"START GAME : S") ;
		TextOutput(0.16,-190.0,25.0," QUIT GAME : Q") ;
		TextOutput(0.16,-190.0,0.0,"     TURNS : LEFT/RIGHT ARROWS") ;
		TextOutput(0.16,-190.0,-25.0," THRUSTERS : UP ARROW") ;
		TextOutput(0.16,-190.0,-50.0,"      FIRE : SPACEBAR") ;
		TextOutput(0.16,-190.0,-75.0,"HYPERSPACE : ESC") ;
		
	}
	
	if ((game_ended) && (!program_started)) {
		glColor3f(1.0,1.0,1.0) ;
		glLineWidth(2.8) ;
		TextOutput(0.20,-100.0,0.0,"GAME OVER") ;
	}
	
	if ((frame_wait > 0) && (game_started)) {
		glColor3f(1.0,1.0,1.0) ;
		glLineWidth(2.8) ;
		TextOutput(0.20,-80.0,230.0,"PLAYER 1") ;
	}
	
	glColor3f(1.0,1.0,1.0) ;
	glPointSize(2.7) ;
	for (i = 0 ; i < MAX_EXPLOSIONS ; i++) {
		if (explosion.debris[i].alive) {
			glBegin(GL_POINTS) ;
			for (j = 0 ; j < explosion.debris[i].num_vertices ; j++) {
				glVertex2f(explosion.debris[i].translation[X] + explosion.debris[i].direction[j][X]*
						   (1.0 - (float)explosion.debris[i].frames/(float)EXPLOSION_FRAMES),
						   explosion.debris[i].translation[Y] + explosion.debris[i].direction[j][Y]*
						   (1.0 - (float)explosion.debris[i].frames/(float)EXPLOSION_FRAMES)) ;
			}
			glEnd() ;
		}
	}
	
	
	glColor3f(0.7, 0.7, 0.7);
	glLineWidth(2.7) ;
	for (i = 0 ; i < asteroids.last ; i++) {
		if (asteroids.rocks[i].alive) {
			glBegin(GL_LINES) ;
			for (j = 0 ; j < asteroids.rocks[i].num_vertices - 1 ; j++) {
				glVertex2f(asteroids.rocks[i].rock[j][X] + asteroids.rocks[i].translation[X],
						   asteroids.rocks[i].rock[j][Y] + asteroids.rocks[i].translation[Y]) ;
				glVertex2f(asteroids.rocks[i].rock[j+1][X] + asteroids.rocks[i].translation[X],
						   asteroids.rocks[i].rock[j+1][Y] + asteroids.rocks[i].translation[Y]) ;
			}
			glEnd() ;
		}
	}
	
	// Wrap large saucer around screen
	if(big_bad.alive)
		BigBadGuyWrapAround();
	
	// If large saucer is on the screen, draw saucer
	if(big_bad.alive && big_bad.on_screen){
		int k;
		glColor3f(0.9,0.9,0.9);
		glLineWidth(1.0);
		glPushMatrix();
		//translate vertices of saucer
		glTranslatef(big_bad.translation[X],big_bad.translation[Y],0.0);
		// draw line segments between vertices of saucer
		glBegin(GL_LINES);
		// loop through line segments of large saucer
		for(k=0;k<big_bad.num_vertices -1;k++){
			glVertex2f(big_bad.saucer[k][X], big_bad.saucer[k][Y]);
			glVertex2f(big_bad.saucer[k+1][X],big_bad.saucer[k+1][Y]);
		}
		glEnd();
		glPopMatrix();
	}
	
	// Wrap small saucer around the screen
	if(small_bad.alive)
		SmallBadGuyWrapAround();
	
	//If small saucer is on the screen, draw saucer
	if(small_bad.alive && small_bad.on_screen){
		int k;
		glColor3f(0.9,0.9,0.9);
		glLineWidth(1.5);
		glPushMatrix();
		// translate vertices of small saucer
		glTranslatef(small_bad.translation[X],small_bad.translation[Y],0.0);
		// draw line segments between vertices of saucer
		glBegin(GL_LINES);
		// loop through line segments of saucer
		for(k=0;k<small_bad.num_vertices -1;k++){
			glVertex2f(small_bad.saucer[k][X], small_bad.saucer[k][Y]);
			glVertex2f(small_bad.saucer[k+1][X],small_bad.saucer[k+1][Y]);
		}
		glEnd();
		glPopMatrix();
	}
	
	
	if ((ship.alive) && (!ship.hyperspaced) && (!ship.exploding)) {
		glColor3f(0.9, 0.9, 0.9) ;
		glLineWidth(3.0) ;
		glPushMatrix() ;
		glTranslatef(ship.translation[X],ship.translation[Y],0.0);
		
		glRotatef(ship.angle, 0.0, 0.0, 1.0) ;
		glBegin(GL_LINES) ;
		for (j = 0 ; j < ship.num_vertices - 1 ; j++) {
			glVertex2f(ship.ship[j][X],ship.ship[j][Y]) ;
			glVertex2f(ship.ship[j+1][X],ship.ship[j+1][Y]) ;
		}
		if (accelerate) {
			blink = ++blink%5 ;
			if (blink <= 2) {
				glVertex2f(4.0,-5.0) ;
				glVertex2f(0.0,-13.0) ;
				glVertex2f(0.0,-13.0) ;
				glVertex2f(-4.0,-5.0) ;
			}
		}
		glEnd() ;
		glPopMatrix() ;
	}
	
	if (ship.exploding) {
		glColor3f(1.0, 1.0, 1.0) ;
		glLineWidth(3.5) ;
		glPushMatrix() ;
		glTranslatef(ship.translation[X],ship.translation[Y],0.0) ;
		glRotatef(ship.angle, 0.0, 0.0, 1.0) ;
		glBegin(GL_LINES) ;
		for (i = 0 ; i < ship.num_vertices - 1 ; i++) {
			if (ship.frames[i] > 0) {
				ship.frames[i]-- ;
				glVertex2f(ship.ship[i][X] + (1 + ship.cur_speed/6.0)*ship.normal[i][X]*(WRECK_WAIT - frame_wait), 
						   ship.ship[i][Y] + (1 + ship.cur_speed/6.0)*ship.normal[i][Y]*(WRECK_WAIT - frame_wait)) ;
				glVertex2f(ship.ship[i+1][X] + (1 + ship.cur_speed/6.0)*ship.normal[i][X]*(WRECK_WAIT - frame_wait),
						   ship.ship[i+1][Y] + (1 + ship.cur_speed/6.0)*ship.normal[i][Y]*(WRECK_WAIT - frame_wait)) ;
			}
		}
		glEnd() ;
		glPopMatrix() ;
	}
	
	glColor3f(0.9, 0.9, 0.9) ;
	glLineWidth(2.0) ;
	for (i = 0 ; i < num_ships ; i++) {
		glPushMatrix() ;
		glTranslatef(-380.0 + 17.0*i,240,0.0) ;
		glBegin(GL_LINES) ;
		for (j = 0 ; j < ship.num_vertices - 1 ; j++) {
			glVertex2f(ship.ship[j][X],ship.ship[j][Y]) ;
			glVertex2f(ship.ship[j+1][X],ship.ship[j+1][Y]) ;
		}
		glEnd() ;
		glPopMatrix() ;
	}
	
	glPointSize(TORPEDO_SIZE) ;
	for (i = 0 ; i < MAX_TORPEDOES ; i++) {
		if (salvo.torpedoes[i].alive) {
			glBegin(GL_POINTS) ;
			for (j = 0 ; j < TORPEDO_STREAK ; j++) {
				glColor3f(1.0 - (float)j/TORPEDO_STREAK, 1.0 - (float)j/TORPEDO_STREAK, 1.0 - (float)j/TORPEDO_STREAK) ; 
				glVertex2f(salvo.torpedoes[i].translation[X] - j*salvo.torpedoes[i].direction[X], 
						   salvo.torpedoes[i].translation[Y] - j*salvo.torpedoes[i].direction[Y]) ;
			}
			glEnd() ;
		}
	}
	glutSwapBuffers() ;
}

/*
 * \name   GenerateNextFrame
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * Method that specifies what to display in following frame on display
 */
void GenerateNextFrame()

{ GLfloat V[COORD], speed ;
	int i, j, last, intersect ;
	
	last = asteroids.last ;
	for (i = 0 ; i < MAX_TORPEDOES ; i++) {
		if (salvo.torpedoes[i].alive) {
			for (j = 0 ; j < last ; j++) {
				if (asteroids.rocks[j].alive) {
					TorpedoAsteroids(i,j) ;
				}
			}
		}
	}
	
	// If big saucer is on the screen
	if((ship.alive) && big_bad.alive && big_bad.on_screen){
		// loop through torpedos
		for(i=0;i<MAX_TORPEDOES ; i++) {
			//if torpedo is live, check if intersects with big saucer
			if( salvo.torpedoes[i].alive) {
				TorpedoBigBadGuy(i);
			}
		}
		
	}
	
	// If ship is on the screen
	if ((ship.alive) && (!ship.hyperspaced) && (!ship.exploding)) {
		// loop through torpedoes
		for(i=0;i<MAX_TORPEDOES ; i++) {
			// if torpedo is live, check if intersects with ship
			if( salvo.torpedoes[i].alive) {
				TorpedoShip(i);
			}
		}
		
	}
	
	// If small saucer is on the screen
	if((ship.alive) && small_bad.alive && small_bad.on_screen){
		// Loop through torpedos
		for(i=0;i<MAX_TORPEDOES ; i++) {
			//if torpedo is live, check if intersects with small saucer
			if( salvo.torpedoes[i].alive) {
				TorpedoSmallBadGuy(i);
			}
		}
		
	}
	if ((ship.alive) && (!ship.hyperspaced) && (!ship.exploding)) {
		last = asteroids.last ;
		for (i = 0 ; i < ship.num_vertices - 1 ; i++) {
			for (j = 0 ; j < last ; j++) {
				if (asteroids.rocks[j].alive) {
					ShipAsteroids(i,j) ;
				}
			}
		} 
	}
	
	// if big saucer is on screen
	if(big_bad.alive && big_bad.on_screen){
		last=asteroids.last;
		//loop through vertices of big saucer
		for(i=0;i<big_bad.num_vertices -1; i++){
			//loop through asteroids
			for(j=0;j<last;j++){
				//if asteroid is live, check if intersects with big saucer
				if(asteroids.rocks[j].alive) {
					BigBadGuyAsteroids(i,j);
				}
			}
		}
	}
	
	// if small saucer is on screen
	if(small_bad.alive && small_bad.on_screen){
		last=asteroids.last;
		//loop through vertices of small saucer
		for(i=0;i<small_bad.num_vertices -1; i++){
			// loop through asteroids
			for(j=0;j<last;j++){
				// if asteroid is live, check if intersects with small saucer
				if(asteroids.rocks[j].alive) {
					SmallBadGuyAsteroids(i,j);
				}
			}
		}
	}
	
	// if ship and large saucer are on screen
	if ((ship.alive) && (!ship.hyperspaced) && (!ship.exploding) && big_bad.alive && big_bad.on_screen) {
		last = asteroids.last ;
		// check if ship intersects with large saucer
		for (i = 0 ; i < ship.num_vertices - 1 ; i++) {
			ShipBigBadGuy(i);
		} 
	}
	
	// if ship and small saucer are on screen
	if ((ship.alive) && (!ship.hyperspaced) && (!ship.exploding) && small_bad.alive && small_bad.on_screen) {
		last = asteroids.last ;
		//check if ship intersects with small saucer
		for (i = 0 ; i < ship.num_vertices - 1 ; i++) {
			ShipSmallBadGuy(i);
		} 
	}
	
	end_round = TRUE ;
	for (i = 0 ; i <  asteroids.last ; i++) {
		if (asteroids.rocks[i].alive) {
			end_round = FALSE ;
			asteroids.rocks[i].translation[X] += asteroids.rocks[i].direction[X]*asteroids.rocks[i].speed ;
			asteroids.rocks[i].translation[Y] += asteroids.rocks[i].direction[Y]*asteroids.rocks[i].speed ;
		}
	}
	
	if ((ship.alive) && (!ship.hyperspaced) && (!ship.exploding)) {
		if (turn_left) {
			ship.angle = ((int)ship.angle + DEG_PER_FRAME)%DEGREES ;
			ship.orientation[X] = -sin(ship.angle*DEG_RAD_CONV) ;
			ship.orientation[Y] = cos(ship.angle*DEG_RAD_CONV) ;
		}
		else { 
			if (turn_right) {
				ship.angle = ((int)ship.angle - DEG_PER_FRAME)%DEGREES ;
				ship.orientation[X] = -sin(ship.angle*DEG_RAD_CONV) ;
				ship.orientation[Y] = cos(ship.angle*DEG_RAD_CONV) ;
			}
		}
		
		if (accelerate) {
			ship.acc_speed = LINEAR_ACCEL ;
			V[X] = ship.orientation[X]*ship.acc_speed + ship.direction[X]*ship.cur_speed ;
			V[Y] = ship.orientation[Y]*ship.acc_speed + ship.direction[Y]*ship.cur_speed ;
			speed = sqrt(pow(V[X],2.0) + pow(V[Y],2.0)) ;
			if (speed > SHIP_SPEED) {
				V[X] = V[X]/speed*SHIP_SPEED ; 
				V[Y] = V[Y]/speed*SHIP_SPEED ; 
				ship.cur_speed = SHIP_SPEED ;
			}
			else {
				ship.cur_speed = speed ;
			}
			ship.direction[X] = V[X]/ship.cur_speed ;
			ship.direction[Y] = V[Y]/ship.cur_speed ;
			ship.translation[X] += V[X] ;
			ship.translation[Y] += V[Y] ;
		}
		else {
			ship.acc_speed = 0.0 ;
			ship.cur_speed -= ship.drag_factor ;
			if (ship.cur_speed < 0.0) {
				ship.cur_speed = 0.0 ;
			}
			ship.translation[X] += ship.direction[X]*ship.cur_speed ;
			ship.translation[Y] += ship.direction[Y]*ship.cur_speed ; 
		}
	}
	//if large saucer is on screen
	if(big_bad.alive && big_bad.on_screen) {
		// translate large saucer
		float incx=big_bad.direction[X]*big_bad.speed;
		big_bad.translation[X] += incx  ;
		big_bad.translation[Y] += big_bad.direction[Y]*big_bad.speed;
		// decrement turn counter
		big_bad.turn_ctr = big_bad.turn_ctr - (int)incx;
		// if turn counter reached zero, assign new direction to large saucer
		if(big_bad.turn_ctr <= 0) 
			BigNewDirection();
	}	
	
	// if small saucer is alive and on screen
	if(small_bad.alive && small_bad.on_screen ) {
		// translate small saucer
		float incx=small_bad.direction[X]*small_bad.speed;
		small_bad.translation[X] += incx  ;
		small_bad.translation[Y] += small_bad.direction[Y]*small_bad.speed;
		// decrement turn counter
		small_bad.turn_ctr = small_bad.turn_ctr - (int)incx;
		// if turn counter reached zero assign new direction to small saucer
		if(small_bad.turn_ctr <= 0) 
			SmallNewDirection();
	}	
	
	
	for (i = 0 ; i < MAX_EXPLOSIONS ; i++) {
		if (explosion.debris[i].alive) {
			explosion.debris[i].frames-- ;
			if (explosion.debris[i].frames == 0) {
				explosion.debris[i].alive = FALSE ;
			}
		}
	}
	
	for (i = 0 ; i < MAX_TORPEDOES ; i++) {
		if (salvo.torpedoes[i].alive) {
			salvo.torpedoes[i].frames-- ;
			if (salvo.torpedoes[i].frames == 0) {
				salvo.torpedoes[i].alive = FALSE ;
			}
			else {
				salvo.torpedoes[i].translation[X] += salvo.torpedoes[i].direction[X]*salvo.torpedoes[i].speed ;
				salvo.torpedoes[i].translation[Y] += salvo.torpedoes[i].direction[Y]*salvo.torpedoes[i].speed ;
			}
		}
	}
	// If big saucer is on screen, check if big saucer should shoot
	if(big_bad.alive && big_bad.on_screen)
		BigShoot();
	//if small saucer is on screen, check if small saucer should shoot
	if( small_bad.alive && small_bad.on_screen)
		SmallShoot();
	if (ship.alive && !ship.hyperspaced && shooting) {
		shooting = FALSE ;
		i = 0 ;
		while ((salvo.torpedoes[i].alive) && (i < MAX_TORPEDOES)) {
			i++ ;
		}
		if (i < MAX_TORPEDOES) {
			salvo.torpedoes[i].alive = TRUE ;
			salvo.torpedoes[i].translation[X] = ship.translation[X] + ship.orientation[X]*SHIP_TIP ;
			salvo.torpedoes[i].translation[Y] = ship.translation[Y] + ship.orientation[Y]*SHIP_TIP ;   
			V[X] = ship.orientation[X]*TORPEDO_SPEED + ship.direction[X]*ship.cur_speed ;
			V[Y] = ship.orientation[Y]*TORPEDO_SPEED + ship.direction[Y]*ship.cur_speed ;
			speed = sqrt(pow(V[X],2.0) + pow(V[Y],2.0)) ;
			salvo.torpedoes[i].direction[X] = V[X]/speed ;
			salvo.torpedoes[i].direction[Y] = V[Y]/speed ;
			salvo.torpedoes[i].speed = speed ;
			if (salvo.torpedoes[i].speed < TORPEDO_MIN_SPEED) salvo.torpedoes[i].speed = TORPEDO_MIN_SPEED ;
			salvo.torpedoes[i].frames = TORPEDO_FRAMES ;
		}
	}
	
	for (i = 0 ; i <  asteroids.last ; i++) {
		AsteroidWrapAround(i) ;
	}
	if ((ship.alive) && (!ship.hyperspaced)) { 
		ShipWrapAround() ;
	}
	if (end_round && frame_wait == 0) {
		frame_wait = FRAME_WAIT ;
	}
	// if round hasn't ended and ship is alive
	else if(ship.alive){
		//if big saucer is not alive, check if should be sent into game
		 if( !big_bad.alive)
			 CheckBigBadGuy();
		//if small saucer is not alive, check if should be sent into game
		if(!small_bad.alive)
			CheckSmallBadGuy();
	}
	TorpedoWrapAround() ;
	glutPostRedisplay() ;
}

/*
* \name   TimerFunction
* \author Manor Freeman 
* \date   2012/10/26
*
* Method called at specific time intervals, checking for events, and calling method to display next frame
*/
void TimerFunction()

{ if (!game_stopped) {
    if (frame_wait > 0) {
		frame_wait-- ;
    }
    if (frame_wait == 0) {
		if (ship.exploding) {
			ship.exploding = FALSE ;
		}
		if (num_ships > 0) {
			if (ship.hyperspaced) {
				ReEntry() ;
				ship.hyperspaced = FALSE ;
			}
			//if new game
			if (game_started) { 
				game_started = FALSE ;
				if (num_rounds < ROUNDS) { 
					num_rounds++ ;
				}
				GenerateRound(&ast_shapes,&asteroids,num_rounds*2 + 2) ; 
			} 
			else {
				if (ship.alive) {
					if (end_round && !big_bad.on_screen && !small_bad.on_screen) { // case where the ship is on screen and waiting for asteroids to come 
						if (num_rounds < ROUNDS) {
							num_rounds++ ;
						}
						
						GenerateRound(&ast_shapes,&asteroids,num_rounds*2 + 2) ; 
					}
				}
				else {
					if (!end_round || (big_bad.on_screen && big_bad.alive) || (small_bad.on_screen && small_bad.alive))
					{ // ship is dead, asteroids are flying 
						if (SuitableTime()) {
							ship.alive = TRUE ;
							ship.waiting = FALSE ;
							ship.cur_speed = 0.0 ;
							ship.translation[X] = 0.0 ;
							ship.translation[Y] = 0.0 ;
						}
					}
					else { // ship is dead, there are no asteroids on screen 
						if (num_rounds < ROUNDS) {
							num_rounds++ ;
						}
						GenerateRound(&ast_shapes,&asteroids,num_rounds*2 + 2) ; 
						ship.alive = TRUE ;
						ship.cur_speed = 0.0 ;
						ship.translation[X] = 0.0 ;
						ship.translation[Y] = 0.0 ;
					}
				}
			}
		}
		
		else {
			// if player got high score - save score
			if (score > high_score) {
				high_score = score ; 
				itoa(high_score,high_score_string) ;
			}
			itoa(score,score_string) ;
			game_ended = TRUE ;
			ship.alive = FALSE ;
			ship.waiting = FALSE ;
			big_bad.alive=FALSE;
			big_bad.wait_started=FALSE;
			big_bad.on_screen=FALSE;
			small_bad.on_screen=FALSE;
			small_bad.alive=FALSE;
			small_bad.wait_started=FALSE;
			ship.cur_speed = 0.0 ;
			ship.translation[X] = 0.0 ;
			ship.translation[Y] = 0.0 ;
		}
    }
    GenerateNextFrame() ;
}
	glutTimerFunc(MSEC,TimerFunction,1) ; 
}

/*
* \name   StartGame
* \author Manor Freeman 
* \date   2012/10/26
*
* Initializes default values for start
*/
void StartGame() 

{ program_started = FALSE ;
	game_ended = FALSE ;
	ship.alive = TRUE ;
	ship.hyperspaced = FALSE ;
	ship.waiting = FALSE ;
	ship.exploding = FALSE ; 
	end_round = TRUE ;
	asteroids.last = 0 ;
	score = 0 ;
	itoa(score,score_string) ;
	extra = 0 ;
	game_started = TRUE ;
	turn_left = FALSE ;
	turn_right = FALSE ;
	accelerate = FALSE ;
	frame_wait = FRAME_WAIT ; 
	num_ships = 4 ;
	num_rounds = 0 ; 
	blink = 0 ;
	round_start=time(0);
}


/**
 * \name   Keyboard
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param char	key pressed
 * \input param x	x-position of mouse
 * \input param y	y-position of mouse
 * Event listener for keyboard
 */
void Keyboard(unsigned char key, int x, int y) 

{ switch(key) {
	//shoot if space pressed
	case ' ':
		if ((ship.alive) && (!ship.hyperspaced)) {
			shooting = TRUE ;
		}
		break ;
	//start game is s pressed
	case 's':
		if (game_ended) {
			StartGame() ;
		}
		break ;
	//quit game if q pressed
	case 'q':
		exit(0) ;
		break ;
	//hyperspace if escape pressed
	case 27: 
		if (ship.alive && !ship.hyperspaced && !ship.exploding && !ship.waiting) {
			HyperSpace() ;
		}
		break ;
	case 'b':
		if (game_stopped) {
			game_stopped = FALSE ;
		}
		else {
			game_stopped = TRUE ;
		}
	default:
		break ;
}
}

/**
 * \name   SpecialKeys
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param key	key pressed
 * \input param x	x-position of mouse
 * \input param y	y-position of mouse
 * Event listener for arrow keys
 */
void SpecialKeys(int key, int x, int y)

{ switch(key) {
	case GLUT_KEY_LEFT:
		turn_left = TRUE ;
		break ;
	case GLUT_KEY_RIGHT:
		turn_right = TRUE ;
		break ;
	case GLUT_KEY_UP:
		accelerate = TRUE ;
	default:
		break ;
}
}

/**
 * \name   SpecialReleaseKeys
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param key	key pressed
 * \input param x	x-position of mouse
 * \input param y	y-position of mouse
 * Event listener for arrow keys
 */
void SpecialReleaseKeys(int key, int x, int y)

{ switch(key) {
	case GLUT_KEY_LEFT:
		turn_left = FALSE ;
		break ;
	case GLUT_KEY_RIGHT:
		turn_right = FALSE ;
		break ;
	case GLUT_KEY_UP:
		accelerate = FALSE ;
		break ;
	default:
		break ;
}
}

/**
 * \name   Reshape
 * \author Manor Freeman
 * \date   2012/10/26
 *
 * \input param w	width of window
 * \input param h	height of window
 * Method handling reshaping of window
 */
void Reshape(int w, int h)

{ glViewport(0,0, (GLsizei) w, (GLsizei) h) ;
	
	glMatrixMode(GL_PROJECTION) ;
	glLoadIdentity() ;
	glOrtho(MIN_X, MAX_X, MIN_Y, MAX_Y, -1.0, 1.0) ;
	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;
}
/*
* \name   main
* \author Manor Freeman and UWO CS3388
* \date   2012/10/26
*
* Main method of program - controls execution of game
* \param argc    number of arguments passed to program
* \param argv    pointer to arguments passed to program
* \returns int   indicating if main executed successfully
*/
int main(argc, argv)
int argc ;
char *argv[] ;

{ time_t t ;
	
	game_stopped = FALSE ;
	game_started = FALSE ;
	game_ended = TRUE ;
	program_started = TRUE ;
	high_score = 0 ;
	score = 0 ;
	itoa(high_score,high_score_string) ;
	time(&t) ;
	srand(t) ;
	glutInit(&argc, argv) ;
	Init() ;
	/* Reading the ship coordinate points */
	GetShip("ship.dat",&ship) ;         
	/* and explosion points */
	GetExplosion("explosion.dat",&explosion) ;   
	/* and asteroid points */
	GetShapes("shapes.dat",&ast_shapes) ;
	// Read large saucer coordinate points
	GetBigBadGuy("saucers.dat",&big_bad);
	// Read small saucer coordinate points
	GetSmallBadGuy("saucers.dat",&small_bad);
	// Assign random directions to saucers
	BigRandomDirection();
	SmallRandomDirection();
	GenerateTorpedoes(&salvo) ;                       /* Arming the torpedoes */
	glutDisplayFunc(RenderFrame) ;      /* OpenGL code for callback functions */
	glutReshapeFunc(Reshape) ;
	glutKeyboardFunc(Keyboard) ;
	glutSpecialFunc(SpecialKeys) ;    /* Processing special keys for the game */
	glutSpecialUpFunc(SpecialReleaseKeys) ;
	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON) ;
	glutIgnoreKeyRepeat(0) ;
	glutMouseFunc(NULL) ;
	
	glutTimerFunc(MSEC,TimerFunction,1) ;
	glutMainLoop() ;                                         /* Play the game */
}
