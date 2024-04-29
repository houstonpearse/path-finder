#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

/*format constants*/
#define MAXRTCELLS 5

/* symbol constants */
#define BLOCKCHAR '#'
#define MORECHAR '$'
#define INCHAR 'I'
#define GCHAR 'G'
#define BLANK ' '
#define ROUTE '*'
#define COORDS '['

/* status constants */
#define NOSTAT 0
#define FLSEIN 1
#define FLSEGO 2
#define ILLM 3
#define BLOCKED 4
#define UNREP 5

/* contant for numgrid in repairrouteseg */
#define EMPTY -1

typedef struct {
    int row, col, depth;
} cell_t;

typedef struct node node_t;
typedef cell_t data_t;

struct node {
    data_t data;
    node_t *next,*prev;
};

typedef struct {
	node_t *head;
	node_t *foot;
} list_t;

typedef struct {
    char **grid;
    int row, col;
} grid_t;

/* function prototypes */
list_t *insert_at_foot(list_t *list, data_t value);
list_t *insert_at_head(list_t *list, data_t value);
list_t *make_empty_list(void);
void free_list(list_t *list);

int check_route(list_t *route, grid_t *gridstate);
void print_route(list_t *route);
void print_status(int status);
void insert_route(list_t *curroute,grid_t *gridstate);
int repair_routeseg(list_t *curroute,grid_t *gridstate);
int route_search(list_t route,cell_t cell);
int repair_route(list_t *curroute,grid_t *gridstate);

grid_t *make_grid(int rows,int columns);
void free_grid(grid_t *gridstate);
void print_grid(grid_t *gridstate);
void remove_char(grid_t *gridstate,char c);
int insert_cell(grid_t *gridstate,cell_t cell,char c);

int get_cell(cell_t *cell);
int check_blocks(list_t *nextblocks,list_t *prevblocks);
	
/*================================================================*/

int
main(int argc, char *argv[]) {
	grid_t *gridstate=NULL;
    list_t *curroute=NULL,*prevblocks,*nextblocks;
    int numblocks=0,numrows,numcolumns,c,status;
    cell_t nextcell,initialcell,goalcell;

    printf("==STAGE 0=======================================\n");
    /* get the dimensions of the grid */
    if (scanf("%dx%d\n",&numrows,&numcolumns) != 2) {
        fprintf(stderr,"failed to read grid dimensions");
        exit(EXIT_FAILURE);
    }

    /* make empty grid */
    gridstate = make_grid(numrows,numcolumns);

    /* get the initial and goal positions */
    get_cell(&initialcell);
    if (!insert_cell(gridstate,initialcell,INCHAR)) {
        fprintf(stderr,"Initial cell outside grid");
        exit(EXIT_FAILURE);
    }  
    get_cell(&goalcell);
    if (!insert_cell(gridstate,goalcell,GCHAR)) {
        fprintf(stderr,"Goal cell outside grid!");
        exit(EXIT_FAILURE);
    }
    
    /* insert blocks */
    prevblocks = make_empty_list();
    while ((c=get_cell(&nextcell)) == 0) {
        if (!insert_cell(gridstate,nextcell,BLOCKCHAR)) {
            fprintf(stderr,"block cell outside grid!");
            exit(EXIT_FAILURE);
        }
        prevblocks = insert_at_foot(prevblocks,nextcell);
        numblocks++;
    }

    /* report inputs given */
    printf("The grid has %d rows and %d columns.\n",numrows,numcolumns);
    printf("The grid has %d block(s).\n",numblocks);
    printf("The initial cell in the grid is [%d,%d].\n",
    initialcell.row,initialcell.col);
    printf("The goal cell in the grid is [%d,%d].\n",
    goalcell.row,goalcell.col);

    /* create route list */
    curroute = make_empty_list();
    while ((c=get_cell(&nextcell)) == 0) {
        curroute = insert_at_foot(curroute,nextcell);
    }

    status = check_route(curroute,gridstate);
    printf("The proposed route in the grid is:\n");
    print_route(curroute);
    print_status(status);
    printf("==STAGE 1=======================================\n");
    if (status != BLOCKED) {
        /* dont need to repair route */
        insert_route(curroute,gridstate);
        print_grid(gridstate);

    } else {
        
        /* write the route into the grid and print the grid */
        insert_route(curroute,gridstate);
        print_grid(gridstate);
        printf("------------------------------------------------\n");

        /* attempt to repair route */
        if (repair_routeseg(curroute,gridstate)==UNREP) {
            /* if route couldnt be repaired */
            print_grid(gridstate);
            printf("------------------------------------------------\n");
            status = UNREP;
            print_status(UNREP);

        } else {
            /*route seg was repaired, wipe grid, insert newroute, print grid*/
            remove_char(gridstate,ROUTE);
            insert_route(curroute,gridstate);
            print_grid(gridstate);
            /* print new ne route with status of */
            printf("------------------------------------------------\n");
            status = check_route(curroute,gridstate);
            print_route(curroute);
            print_status(status);
        }
    }

    /* continue with stage 2 if statuses BLOCKED UNREP NOSTAT */
    if ( (c!=EOF) && ((status == BLOCKED) || (status == UNREP) ||
    	(status == NOSTAT))) {
        printf("==STAGE 2=======================================\n");
        /* if we are still blocked from stage 1 finish repair */
        if (status==BLOCKED) {
            /* repair route */
            status = repair_route(curroute,gridstate);
            /* show old route */
            print_grid(gridstate);
            printf("------------------------------------------------\n");
            /* show new route */
            remove_char(gridstate,ROUTE);
            insert_route(curroute,gridstate);
            print_grid(gridstate);
            printf("------------------------------------------------\n");
            /* show status */
            print_route(curroute);
            print_status(status);
            printf("================================================\n");

        }

        /* while there are more blocks to be loaded */
        while (c==MORECHAR) {
            /* wipe gridstate */
            remove_char(gridstate,ROUTE);
            remove_char(gridstate,BLOCKCHAR);
            
            /* insert new blocks */
            nextblocks = make_empty_list();
            while ((c=get_cell(&nextcell)) == 0) {
                if (!insert_cell(gridstate,nextcell,BLOCKCHAR)) {
                    fprintf(stderr,"new block cell incorrect!\n");
                    exit(EXIT_FAILURE);
                }
                insert_at_foot(nextblocks,nextcell);
            }

            /* if newset of blocks == old set skip to next*/
            if (check_blocks(nextblocks,prevblocks)==0) {

                /* insert route */
                insert_route(curroute,gridstate);
                print_grid(gridstate);
                if (check_route(curroute,gridstate)!=NOSTAT) {
                    /* if the new route needs repairing */
                    printf("------------------------------------------------");
                    printf("\n");
                    remove_char(gridstate,ROUTE);
                    status = repair_route(curroute,gridstate);
                    insert_route(curroute,gridstate);
                    print_grid(gridstate);
                    printf("------------------------------------------------");
                    printf("\n");
                    if (status == NOSTAT) {
                        print_route(curroute);
                    }
                    print_status(status);
                }
                printf("================================================\n");
            }
            free_list(prevblocks);
            prevblocks = nextblocks;
        }
    } else {
        /* other types of error that we cannot proceed with */
        printf("================================================\n");
    }
    
    
    free_list(prevblocks);
    free_grid(gridstate);
    free_list(curroute);
	return 0;
}
/* algorithms are fun */

/*================================================================*/
/* check if the blocks from two blocks lists are the same */
int check_blocks(list_t *nextblocks,list_t *prevblocks) {
    node_t *nextblock=nextblocks->head,*prevblock=prevblocks->head;

    while ((nextblock!=NULL) && (prevblock!=NULL)) {
        
        /* if a node in one doesnt appear in the other, they dont match */
        if (route_search(*nextblocks,prevblock->data)==false) {
            return false;
        } else if (route_search(*prevblocks,nextblock->data)==false) {
            return false;
        }

        nextblock = nextblock->next;
        prevblock = prevblock->next;
    }

    if ((nextblock==NULL) && (prevblock ==NULL)) {
        /* we have checked all items */
        return true;
    } else {
        /* there is more items in one*/
        /* check these items are in the other list */
        if (nextblock) {
            /* excess in nextblocks, check items are in prevblocks */
            while (nextblock) {
                if (route_search(*prevblocks,nextblock->data)==false) {
                    return false;
                }
                nextblock = nextblock->next;
            }

            return true;

        } else {
            /* excess in prevblocks, check they are in nextblocks */
            while (prevblock) {
                if (route_search(*nextblocks,prevblock->data)==false) {
                    return false;
                }
                prevblock = prevblock->next;
            }
            return true;
        }
    }
}

/*================================================================*/
/* creates whitespace filled grid and returns it */

grid_t
*make_grid(int rows, int columns) {
    grid_t *gridstate;
    char tempstr[columns+1];
    int i;

    /* make a default row to insert into */
    for (i=0;i<columns;i++) {
        tempstr[i] = BLANK;
    }
    tempstr[columns] = '\0';
    
    /* allocate space for memory and insert dimensions */   
    gridstate = (grid_t*)malloc(sizeof(*gridstate));
    assert(gridstate != NULL);
    gridstate->row = rows;
    gridstate->col = columns;

    /* allocate memory for the grid */
    gridstate->grid = (char**)malloc(rows*sizeof(*(gridstate->grid)));
    assert(gridstate->grid != NULL);
    
    /* fill the grid with blanks */
    for (i=0;i<rows;i++) {
        (gridstate->grid)[i] = (char*)malloc(columns*sizeof(char)+1);
        assert((gridstate->grid)[i] != NULL);
        strcpy((gridstate->grid)[i],tempstr);
    }
    return gridstate;
}

/*================================================================*/
/* a funcion that frees the memory allocated to the grid */

void
free_grid(grid_t *gridstate) {
    int i;
    /* free strings in array */
    for (i=0;i<gridstate->row;i++) {
        free(gridstate->grid[i]);
    }
    /* free  grid array and grid struct */
    free(gridstate->grid);
    free(gridstate);
}

/*================================================================*/
/* a function that prints out the grid */

void
print_grid(grid_t *gridstate) {
    int i,j=0;
    
    /* row of single digit numbers starting at 0 show grid size */
    printf(" ");
    for (i=0;i<gridstate->col;i++) {
        /* if we get to 10 start at 0 again to print single digits */
        if (j==10) {
            j=0;
        }
        printf("%d",j);
        j++;
    }
    printf("\n");
    
    /* print grid rows with 1 digit row numbers in front starting at 0*/
    j=0;
    for (i=0;i<gridstate->row;i++) {
        if (j==10) {
            j=0;
        }
        printf("%d%s\n",j,gridstate->grid[i]);
        j++;
    }

}

/*================================================================*/
/* removes all instances of a char c from the grid */

void remove_char(grid_t *gridstate,char c) {
    int i=0,j=0;

    for (i=0;i<gridstate->row;i++) {
        for (j=0;j<gridstate->col;j++) {
            if (gridstate->grid[i][j]==c) {
                gridstate->grid[i][j] = BLANK;
            }
        }
    }
}

/*================================================================*/
/* gets cell from stdin. statusflag: if sucess returns 0, 
if failed returns last char */

int
get_cell(cell_t *cell) {
    int c;
    /* skip to the next cell start or $ */
    while (((c=getchar()) != MORECHAR) && (c != COORDS) && (c !=EOF));

    /* if we are at the end of a cell group or at the end of file */
    if ((c == MORECHAR) || (c == EOF)) {
        return c;
    }

    /* we found a cell */
    scanf("%d,%d",&(cell->row),&(cell->col));
    return 0;

}

/*================================================================*/
/* function inserts char at cell in grid. statusflag: 1 sucess, 0 failure */

int
insert_cell(grid_t *gridstate,cell_t cell,char c) {
    if ((gridstate->col>cell.col) && (gridstate->row>cell.row)) {
        if (((gridstate->grid)[cell.row][cell.col]==INCHAR) || 
        ((gridstate->grid)[cell.row][cell.col]==GCHAR)) {
            fprintf(stderr,"Cannot change initial or goal cells!\n");
            exit(EXIT_FAILURE);
        }
        (gridstate->grid)[cell.row][cell.col] = c;
        
        return 1;
    }
    return 0;
}

/*================================================================*/
/* a function that prints a route status */

void print_status(int status) {
    if (status==FLSEIN) {
        printf("Initial cell in the route is wrong!\n");
    } else if (status==FLSEGO) {
        printf("Goal cell in the route is wrong!\n");
    } else if (status==ILLM) {
        printf("There is an illegal move in this route!\n");
    } else if (status==BLOCKED) {
        printf("There is a block on this route!\n");
    } else if (status==NOSTAT){
        printf("The route is valid!\n");
    } else if (status == UNREP) {
        printf("The route cannot be repaired!\n");
    }
}
/*================================================================*/
/* a function that checks a route returns a status */

int check_route(list_t *route,grid_t *gridstate) {
    int rows,cols,status=NOSTAT,diff,rdiff,cdiff;
    cell_t head,foot;
    node_t *nextnode,*prevnode;

    /* preacessing variables for compactness */
    rows=gridstate->row;
    cols=gridstate->col;
    head=route->head->data;
    foot=route->foot->data;

    /* check if initial and goal cells are valid */
    if ((0<=head.row)&&(head.row<rows)&&(0<=head.col)&&(head.col<cols)) { 
        /* check if the first cell in the route matches the start cell */
        if (gridstate->grid[head.row][head.col]!=INCHAR) {
            status=FLSEIN;
            return status;
        }
    } else {
        /*first cell in the route is outside grid */
        status=FLSEIN;
        return status;
    }

    if ((0<=foot.row)&&(foot.row<rows)&&(0<=foot.col)&&(foot.col<cols)) {
        /* does the last cell in route match the goal cell */
        if (gridstate->grid[foot.row][foot.col]!=GCHAR) {
            status=FLSEGO;
            return status;
        }
    } else {
        /*last cell is outside grid */
        status=FLSEGO;
        return status;
    }

    /* check if the rest of the route is valid */
    nextnode=prevnode=route->head;
    while (nextnode) {

        /*check if nextnode can fit in grid */
        if ((0>nextnode->data.row)||(rows<=nextnode->data.row)) {
            status = ILLM;
            return status;
        } else if ((0>nextnode->data.col)||(cols<=nextnode->data.col)) {
            status = ILLM;
            return status;
        } else {
            /* check if there is a block on the route */
            if (gridstate->grid[nextnode->data.row][nextnode->data.col] == 
            	BLOCKCHAR) {
                status=BLOCKED;
            }
        }

        /* calc (x1-x2)^2 + (y1-y2)^2 = rdiff */
        rdiff = nextnode->data.row-prevnode->data.row;
        cdiff = nextnode->data.col-prevnode->data.col;
        diff = rdiff*rdiff+cdiff*cdiff;

        /* can only be adjacent if (x1-x2)^2 + (y1-y2)^2 <= 1 */
        if (diff>1) {
            status=ILLM;
            return status;
        }

        prevnode = nextnode;
        nextnode = nextnode->next;
    }

    return status;

}

/*================================================================*/
/* a function that prints a route */
void print_route(list_t *route) {
    node_t *nextnode;
    int i=0;

    /* traverse the route list printing out each cell*/
    nextnode=route->head;
    while (nextnode) {
        /* print cell */
        printf("[%d,%d]",nextnode->data.row,nextnode->data.col);
        i++;
        /* determine the suffix needed */
        if ((nextnode->next)==NULL) {
            /* if at last cell */
            printf(".\n");
        } else if (i==MAXRTCELLS) {
            /* if at cell per line limit */
            printf("->\n");
            i=0;
        } else {
            printf("->");
        }

        nextnode = nextnode->next;
    }

}

/*================================================================*/
/* inserts a route into the grid, route must be checked before function call*/

void insert_route(list_t *curroute,grid_t *gridstate) {
    node_t *nextnode;

    /* go through route tracing the path into the grid */
    nextnode=curroute->head;
    while (nextnode) {
        if (gridstate->grid[nextnode->data.row][nextnode->data.col]==BLANK) {
            insert_cell(gridstate,nextnode->data,ROUTE);
        }
        nextnode = nextnode->next;
    }
}

/*================================================================*/
/* repairs first break in route. status: 1 = success, UNREP = unrepairable */

int repair_routeseg(list_t *curroute,grid_t *gridstate) {
    list_t *repairlist=NULL,*bridge=NULL,brokenseg;
    node_t *nextnode=NULL,*firstnode=NULL;
    cell_t tempcell;
    int inblock=false,depth=0,connect=false,**numgrid,i,j;
    
    /* make space for our queue and bridge */
    repairlist = make_empty_list();
    bridge = make_empty_list();
    
    /*create a 2d integer array of the same size as the grid */
    numgrid = (int**)malloc((gridstate->row)*sizeof(int*));
    assert(numgrid != NULL);
    for (i=0;i<gridstate->row;i++) {
        numgrid[i] = (int*)malloc((gridstate->col)*sizeof(int));
        assert(numgrid[i]!=NULL);
        for (j=0;j<gridstate->col;j++) {
            numgrid[i][j]=-1;
        }
    }

    /* find first broken segment */
    nextnode = curroute->head;
    while (nextnode) {
        /* add cell before first block to the queue with depth 0 */
        if ((gridstate->grid[nextnode->data.row][nextnode->data.col] == 
        	BLOCKCHAR) && (inblock==false)) {
            /* our path has been bloked */
            inblock=true;
            /* store cell before block in temp cell with depth 0*/
            tempcell.row = nextnode->prev->data.row;
            tempcell.col = nextnode->prev->data.col;
            tempcell.depth = 0;
            /* store a pointer to the first node before block */
            firstnode = nextnode->prev;
            /* insert node into queue and numgrid */
            repairlist = insert_at_foot(repairlist,tempcell);
            numgrid[tempcell.row][tempcell.col]=0;

        }
        if (inblock==true) {
            if (gridstate->grid[nextnode->data.row][nextnode->data.col] !=
            	BLOCKCHAR) {
                /* we are at the first cell outside of a block */
                /* broken seg contains list of the rest of the 
                route after a block */
                brokenseg.foot = curroute->foot;
                brokenseg.head = nextnode;
                break;
            }
        }
        nextnode = nextnode->next;
    }

    /* flood the grid */
    nextnode = repairlist->head;
    while (nextnode) {
    	
        /*check if the first cell in the queue connects to the route */
        tempcell = nextnode->data;
        if (route_search(brokenseg,tempcell)==true) {
            break;
        }

        /* add cells of next depth */  
        /* up */
        if ((nextnode->data.row-1>=0) && 
        (gridstate->grid[nextnode->data.row-1][nextnode->data.col] != 
        BLOCKCHAR)) {
        
            tempcell = nextnode->data;
            tempcell.row = tempcell.row -1;
            tempcell.depth = tempcell.depth +1;
            
            /* if the next cell being added is allready in the repairlist */
            if (route_search(*repairlist,tempcell)==false) {
                repairlist = insert_at_foot(repairlist,tempcell);
                numgrid[tempcell.row][tempcell.col]=tempcell.depth;
            }
            /* test if the newly added item in q connects to the brokenseg */
            if (route_search(brokenseg,tempcell)==true) {
                break;
            }
        }
        
        /*down*/
        if ((nextnode->data.row+1<gridstate->row) &&
        (gridstate->grid[nextnode->data.row+1][nextnode->data.col] != 
        BLOCKCHAR)) {
        
            tempcell = nextnode->data;
            tempcell.row = tempcell.row +1;
            tempcell.depth = tempcell.depth +1;
            
            /* if the next cell being added is allready in the repairlist */
            if (route_search(*repairlist,tempcell)==false) {
                repairlist = insert_at_foot(repairlist,tempcell);
                numgrid[tempcell.row][tempcell.col]=tempcell.depth;
            }
            /* test if the newly added item in q connects to the brokenseg */
            if (route_search(brokenseg,tempcell)==true) {
                break;
            }
        
        }

        /*left*/
        if ((nextnode->data.col-1>=0) && 
        (gridstate->grid[nextnode->data.row][nextnode->data.col-1] != 
        BLOCKCHAR)) {
        
            tempcell = nextnode->data;
            tempcell.col = tempcell.col -1;
            tempcell.depth = tempcell.depth +1;
            
            /* if the next cell being added is allready in the repairlist */
            if (route_search(*repairlist,tempcell)==false) {
                repairlist = insert_at_foot(repairlist,tempcell);
                numgrid[tempcell.row][tempcell.col]=tempcell.depth;
            }
            /* test if the newly added item in q connects to the brokenseg */
            if (route_search(brokenseg,tempcell)==true) {
                break;
            }
        }

        /*right*/
        if ((nextnode->data.col+1<gridstate->col) && 
        (gridstate->grid[nextnode->data.row][nextnode->data.col+1] != 
        BLOCKCHAR)) {
        
            tempcell = nextnode->data;
            tempcell.col = tempcell.col +1;
            tempcell.depth = tempcell.depth +1;
            
            /* if the next cell being added is allready in the repairlist */
            if (route_search(*repairlist,tempcell)==false) {
                repairlist = insert_at_foot(repairlist,tempcell);
                numgrid[tempcell.row][tempcell.col]=tempcell.depth;
            }
            /* test if the newly added item in q connects to the brokenseg */
            if (route_search(brokenseg,tempcell)==true) {
                break;
            }
        }

        nextnode = nextnode->next;
    }
    /* dont need repairlist all info stored in numgrid */
    free_list(repairlist);

    if (nextnode == NULL) {
        /*unable to repair route return UNREP status */
        free_list(bridge);
        for (i=0;i<gridstate->row;i++) {
        free(numgrid[i]);       
        }
        free(numgrid);
        return UNREP;
    }

    /* we connected the route tempcell is the cell that connected it */
    depth = tempcell.depth;
    bridge = insert_at_head(bridge,tempcell);
    while (depth) {
        
        /*work back through the flooded numgrid to make new path*/
        if ((tempcell.row-1>=0) && 
        (numgrid[tempcell.row-1][tempcell.col]<depth) && 
        (numgrid[tempcell.row-1][tempcell.col]!=EMPTY)) {
            /*up*/
            tempcell.row = tempcell.row-1;
            bridge = insert_at_head(bridge,tempcell);

        } else if ((tempcell.row+1<gridstate->row) && 
        (numgrid[tempcell.row+1][tempcell.col]<depth) && 
        (numgrid[tempcell.row+1][tempcell.col]!=EMPTY)) {
            /*down*/
            tempcell.row = tempcell.row+1;
            bridge = insert_at_head(bridge,tempcell);

        } else if ((tempcell.col-1>=0) && 
        (numgrid[tempcell.row][tempcell.col-1]<depth) && 
        (numgrid[tempcell.row][tempcell.col-1]!=EMPTY)){
            /*left*/
            tempcell.col = tempcell.col-1;
            bridge = insert_at_head(bridge,tempcell);

        } else {
            /*right*/
            tempcell.col = tempcell.col+1;
            bridge = insert_at_head(bridge,tempcell);
        }
        depth--;
    }

    /* connect the bridge to the original path */
    nextnode=curroute->head;
    while (nextnode) {
        /* find the node before the break stored in firstnode */
        if (nextnode==firstnode) {
            /* start of the bridge found. connect with route */ 
            if (nextnode->prev != NULL) {
                nextnode->prev->next =bridge->head;
            } else {
                curroute->head = bridge->head;
            }
            bridge->head->prev = nextnode->prev;
            
            connect = true;
        } else if ((nextnode->data.row==bridge->foot->data.row) &&
            (nextnode->data.col==bridge->foot->data.col) &&
            (connect==true)) {
            /* end of bridge found. connect with route */
            bridge->foot->next=nextnode->next;
            if (nextnode->next != NULL) {
                nextnode->next->prev=bridge->foot;
            } else {
                curroute->foot = bridge->foot;
            }
            free(nextnode);
            
            /* has been connected break the loop */
            break;
        }
        
        nextnode=nextnode->next;
        if ((connect) && (nextnode->prev != NULL)) {
            free(nextnode->prev);
        }
    }
    

    /* free variables*/
    for (i=0;i<gridstate->row;i++) {
        free(numgrid[i]);       
    }
    free(numgrid);
    bridge->foot = NULL;
    bridge->head = NULL;
    free(bridge);

    return 1;
}

/*================================================================*/
/* search for a cell in a route,return 1 if found*/
int route_search(list_t route,cell_t cell) {
    node_t *nextnode;
    nextnode=route.head;
    while (nextnode) {
        /* if the nextcell has the same coords as the test cell*/
        if ((nextnode->data.row==cell.row)&& (nextnode->data.col==cell.col)) {
            return true;
        }
        nextnode = nextnode->next;
    }

    return false;
}

/*================================================================*/
/* try to repair entire returns a status indicating success/failure */

int repair_route(list_t *curroute,grid_t *gridstate) {
    int status=check_route(curroute,gridstate);
    while (status == BLOCKED) {
        if (repair_routeseg(curroute,gridstate)==UNREP) {
            return UNREP;
        } else {
            status = check_route(curroute,gridstate);
        }
    }
    return NOSTAT;
}

/*================================================================*/
/* slighly modified functions used from listops.c in 
 * "Programming, Problem Solving, and Abstraction with C"
*/

list_t
*make_empty_list(void) {
	list_t *list;
	list = (list_t*)malloc(sizeof(*list));
	assert(list!=NULL);
	list->head = list->foot = NULL;
	return list;
}

list_t
*insert_at_foot(list_t *list, data_t value) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	new->data = value;
	new->next = NULL;
    new->prev = NULL;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->head = list->foot = new;
	} else {
		new->prev =list->foot;
        list->foot->next = new;
		list->foot = new;
	}
	return list;
}

list_t
*insert_at_head(list_t *list, data_t value) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	new->data = value;
	new->next = list->head;
    new->prev = NULL;
	if (list->head != NULL) {
        list->head->prev = new;
    }
    
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->foot = new;
	}
    list->head = new;
	return list;
}

void
free_list(list_t *list) {
	node_t *curr, *prev;
	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	free(list);
}

/* algorithms are fun */