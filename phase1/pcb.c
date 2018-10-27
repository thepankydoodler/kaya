#include "../h/const.h"
#include "../h/types.h"

#include "/usr/include/uarm/libuarm.h"
#include "../e/pcb.e"
#include "../e/asl.e"

#define TRUE 1
#define FALSE 0

/* PCB.C */

HIDDEN pcb_PTR pcbFree_h;


void freePcb (pcb_PTR p){
	insertProcQ(&pcbFree_h, p);
}

pcb_PTR allocPcb (){
	pcb_PTR tmp = removeProcQ(&pcbFree_h);
	
	tmp->p_next = NULL;  /* initialize fields */
	tmp->p_prev = NULL;
	tmp->p_prnt = NULL;
	tmp->p_child = NULL;
	tmp->p_sib = NULL;
	tmp->p_prevSib = NULL;
	tmp->p_semAdd = NULL;  

	if(tmp != NULL) return tmp;
	return NULL;
}

void initPcbs (){
	static pcb_PTR foo[MAXPROC];	
	int i;
	for (i = 0; i < MAXPROC; i++) {
		foo[i] = mkEmptyProcQ();
		freePcb(foo[i]);
	}
}



pcb_PTR mkEmptyProcQ (){
	return NULL;
}

int emptyProcQ (pcb_PTR tp){
	return (tp == NULL);
}

/* cases: 1) empty procQ, 2) non-empty procQ */
void insertProcQ (pcb_PTR *tp, pcb_PTR p){
	if (emptyProcQ(*tp)) { /* empty q case 1 */
		*tp = p;
		p->p_next = p;
		p->p_prev = p;
		return;
	}
	/* non-empty q */
	else { /* case 2 */
		pcb_PTR tail = (*tp);
		p->p_next = tail->p_next; /* set next of p */
		p->p_prev = tail; /* set prev of p */
		tail->p_next = p; /* set next of previous tail */
		tail->p_next->p_prev = p; /* set prev of head */
	}
	/* set tail pointer */
	(*tp) = p;
}

/* 2 cases: the q is empty so return null, or 2, remove first element */
pcb_PTR removeProcQ (pcb_PTR *tp){
	if (emptyProcQ(*tp)) { /* case 1*/
		return NULL;
	}
	return outProcQ(tp, *tp); /* case 2 */
}

/* four conditions to account for: 1) p is only pcb in procQ, 2) more than one pcb in procQ and target pcb is first one, 3) it's one of more than one and isn't the first, 4) or it's not there */
pcb_PTR outProcQ (pcb_PTR *tp, pcb_PTR p){
	pcb_PTR firstPcb = *tp;
	if (firstPcb == p) { /* first pcb is p */
		if (p->p_next == p) { /* case 1: p is only pcb on the procQ tp */
			(*tp) = NULL; /* set the tp to null to indicate an empty procQ */
			return p;
		}
		/* condition 2 */
		p->p_prev->p_next = p->p_next; /*adjust next pointer for new tail of procQ */
		p->p_next->p_prev = p->p_prev; /* adjust prev pointer for head of procQ */
		*tp = p->p_prev; /* adjust tp for procQ */
	}
	/* condition 3 */
	pcb_PTR current = firstPcb->p_next; /* current is now head pcb of procQ */
	while (current != firstPcb) { /*while current != tail pcb, i.e. the first one we checked */
		if (current == p) {  /* find right pcb then... */
			p->p_prev->p_next = p->p_next; /* redo next and prev pointers for nodes adjacent to p */
			p->p_next->p_prev = p->p_prev;
			return p;
		}		
		current = current->p_next;
	}
	/* case 3: target pcb not found in procQ */
	return NULL;		
}

pcb_PTR headProcQ (pcb_PTR tp){
	if (emptyProcQ(tp)) {
		return NULL;
	}
	return (tp);
}

int emptyChild (pcb_PTR p){
	return (p->p_child == NULL);
}

/* assumption: it's fine to insert new child at the head of the child list and it's fine to have child lists singly linked */
/* cases: 1) empty child list, 2) non-empty child list */
void insertChild (pcb_PTR prnt, pcb_PTR p){
	if (emptyChild(prnt)) { /* case 1 */
		prnt->p_child = p; /* set p as child or prnt */
		return;
	}
	/*case 2*/
	pcb_PTR firstChild = prnt->p_child;
	prnt->p_child = p; /* set p as new first child of prnt */
	p->p_sib = firstChild;
	firstChild->p_prevSib = p; /*adjust original first child's prev ptr */
}

pcb_PTR removeChild (pcb_PTR p){
	return outChild(p->p_child);
}

/* five conditions to account for: 1) p has no parent, 2) p is only child of its parent, 3) more than one pcb in child list and target p is first one, 4) it's one of more than one and isn't the first */
pcb_PTR outChild (pcb_PTR p){ 
	if (p->p_prnt == NULL) { /* case 1 */
		return NULL;
	}
	if (p->p_prnt->p_child == p) { /* if p is the first child, either falls into case 2 or 3 */
		if (p->p_sib == NULL) { /* case 2 - p is only child */
			p->p_prnt->p_child = NULL; /* no more children of its parent */
			return p;
		}
		/* case 3 */
		p->p_prnt->p_child = p->p_sib; /* next child is now first child */
		p->p_prnt->p_child->p_prevSib = NULL;
		return p;
	}
	/* case 4, we know p isn't first child of its parent, so either p is end of child list or it's not */
	if (p->p_sib == NULL) { /* p is last node on child list */
		p->p_prevSib->p_sib == NULL;
		return p;
	}
	/* still case 4, p is somewhere in the middle of the child list */
	p->p_prevSib->p_sib = p->p_sib; /* adjust prev and next pointers of p's next and prev, respectively */
	p->p_sib->p_prevSib = p->p_prevSib;
	return p;
}



