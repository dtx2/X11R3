
#include "xedit.h"

#define PieceSize(qp)  (qp->endPos - qp->begPos)

typedef struct _qp{
    struct _qp *flink, *blink;
    XtTextSourceRec **src;
    int begPos, endPos;
} PieceQueueElement;
 
typedef struct _cb{
    struct _cb *flink, *blink;
    PieceQueueElement *retiredQHead, *retiredQTail;
    PieceQueueElement *addedPtrFirst, *addedPtrLast, *where, *Rptr, *Lptr;
    XtTextPosition Rfix, Lfix, deletedPos;
}ChangeBlock;
 
typedef struct _XtSourceDataRec{
    XtTextSourceRec *ro, *ao, *source;
    PieceQueueElement *PTQHead, *PTQTail, *lastAddedPiece, *LocatedPiece;
    XtTextPosition endPos, aoPos, lastReplaceEndPos, LocatedPiecePos;
    ChangeBlock *CQHead, *CQTail, *LastUndid;
    XtTextPosition left, right; /* Left and right extents of selection. */
    void (*applicationCallback)(); /* tell an application that we're changed */
} PSContext;
 
static PieceQueueElement *getQp(){
    return((PieceQueueElement *)calloc(sizeof(PieceQueueElement), 1));
}
 
static PieceQueueElement *remqueue(head, tail, this)
  PieceQueueElement **head, **tail, *this;
{
  PieceQueueElement *prev = this->blink;
  PieceQueueElement *next = this->flink;
	if (this == *head){
	  if(this == *tail) {
	    *head = 0;
	    *tail = 0;
	  } else {
	    *head = next;
	    next->blink = 0;
	  }
	} else {
	  if(this == *tail){
	    *tail = prev;
	    prev->flink = 0;   
	  } else {
	    prev->flink = next;
	    next->blink = prev;
	  }
	}
	this->blink = 0;
	this->flink = 0;
	return (this);
}
 
static freeQp(this)
  PieceQueueElement* this;
{
    free((int*)this);
}
 
/*
 * insert 'new' queue element on the queue.  
 */
static PieceQueueElement *insert(head, tail, new, where)
  PieceQueueElement **head, **tail, *new, *where;
{
	if(!where){
	    new->flink = *head;
	    *head = new;
	} else {
	    new->flink = where->flink;
	    new->blink = where;
	    where->flink = new;
	}
	if(new->flink == 0)
	    *tail = new;
	else
	    new->flink->blink = new;
	return(new);
}

/*
 * Old pieces never die; they are retired onto change blocks.  
 */
retire(ctx, qp)
  PSContext *ctx;
  PieceQueueElement *qp;
{
	remqueue(&ctx->PTQHead, &ctx->PTQTail, qp);
	insert(&(ctx->CQTail->retiredQHead), &(ctx->CQTail->retiredQTail),
		qp, ctx->CQTail->retiredQTail);
	ctx->endPos -= (PieceSize(qp));
}
/*
 * Translate a source position into a piece pointer.  Also, return the offset
 * of that position within the piece.  
 */

static PieceQueueElement *locate(ctx, skew, pos)
  XtTextPosition pos;
  int *skew;
  PSContext *ctx;
{
/* referential locality  optimizations go here */
  XtTextPosition t;
  PieceQueueElement *qp;
/*printf("locate %d  %d\n", total++, pos); */
    if ((pos == ctx->endPos) && (ctx->PTQTail)){
	qp = ctx->PTQTail;
	*skew = PieceSize(qp);
        ctx->LocatedPiece = qp;
        ctx->LocatedPiecePos = pos - *skew;
	return qp;
	}
    if(	(ctx->LocatedPiece) && 
	(qp = ctx->LocatedPiece->flink) &&
	(pos >= (t = ctx->LocatedPiecePos + PieceSize(ctx->LocatedPiece))) && 
	(pos < PieceSize(qp) + t)){
	    /* do nothing more! */
    } else {
         t = 0;
        for(qp = ctx->PTQHead; qp; qp = qp->flink){
	    if (t + PieceSize(qp) > pos)
	        break;
	    t += (PieceSize(qp));
        }
    }
    *skew = pos - t;
    ctx->LocatedPiece = qp; 
    ctx->LocatedPiecePos = t;
    return qp;
}
 
/*
 * Split begin and end pieces and delete if necessary unused pieces. 
 * return queue pointer of piece to insert at. 
 */
PieceQueueElement *deleteRange(ctx, startPos, endPos)
  PSContext *ctx;
  XtTextPosition startPos, endPos;
{
  PieceQueueElement *qp, *beg, *end, *victim;
  XtTextPosition t, skew, range = endPos - startPos;
  ChangeBlock *cb = ctx->CQTail;
    if(endPos == 0)
	return 0;
    qp = locate(ctx, &skew, startPos);
    t = startPos - skew;
    if (!skew){
	if( t + PieceSize(qp) == endPos){
	    /* desired range is contained in the one piece exactly */
	    beg = qp->blink;
	    retire(ctx, qp);	    	    
	    return beg;
	} else if( t + PieceSize(qp) > endPos){
	    /* desired range is front portion of one piece */
	    beg = qp->blink;
	    cb->Rptr = qp;
	    cb->Rfix = qp->begPos;
	    qp->begPos += range;	  
	    ctx->endPos -= range;
	    return beg;
	}
    } else { 
	if( t + PieceSize(qp) == endPos){
	    /* range is the back portion of one piece */
	    cb->Lptr = qp;
	    cb->Lfix = qp->endPos;
	    qp->endPos -= range;
	    ctx->endPos -= range;
	    return qp;
	}
    }
    if(skew){
	if( t + PieceSize(qp) < endPos){
	    /* deleted range extends off this piece, so lop off end of it */
	    t += PieceSize(qp);
	    beg = qp->flink;
	    cb->Lptr = qp;
	    cb->Lfix = qp->endPos;
	    ctx->endPos -= PieceSize(qp) - skew;
	    qp->endPos = qp->begPos + skew;
	} else {
	    /*      enter a new piece by splitting the first piece */
	    beg = insert(&ctx->PTQHead, &ctx->PTQTail, getQp(), qp);
	    beg->endPos = qp->endPos;
	    cb->Lfix = qp->endPos;
	    cb->Lptr = qp;
	    qp->endPos = qp->begPos + (startPos - t);
	    beg->begPos = qp->endPos;
	    beg->src = qp->src;
	    cb->addedPtrFirst = cb->addedPtrLast = beg;
            if (!range)
	        return (qp);
            t += PieceSize(qp); /* bump 't' up to beg*/
            if( t + PieceSize(beg) > endPos){
	        /* range size < sizof(beg) */
                ctx->endPos -= (endPos - t);
                beg->begPos += (endPos - t);
	        return qp;
	    }
	}
    } else 
	beg = qp;
/* locate queue entry that contains the last position, deleting others on the way*/
    victim = 0;
    for(qp = beg; qp;  qp = qp->flink){
	if (victim){
	    retire(ctx, victim);
	    victim = 0;
	}
	if( t + (PieceSize(qp)) >= endPos) 
	    break;
	t  += (PieceSize(qp));
	victim = qp;
    }
    if( t + (PieceSize(qp)) == endPos){
	end = qp->blink;
	retire(ctx, qp);
	return (end);
    }
/* implied split and delete first half of last piece */
    cb->Rfix = qp->begPos; 
    cb->Rptr = qp;
    ctx->endPos -= (endPos - t);
    qp->begPos += (endPos - t);
    return (qp->blink);
}
 

#define RightPiece(qp, s2Pos) 		\
{					\
    s2Pos += (PieceSize(qp)); \
    qp = qp->flink;			\
}
 
#define LeftPiece(qp, s2Pos)		\
{					\
    qp = qp->blink;			\
    if(qp)				\
        s2Pos -= (PieceSize(qp));	\
}


#define Increment(data, position, direction)\
{\
    if (direction == XtsdLeft) {\
	if (position > 0) \
	    position--;\
    }\
    else {\
	if (position < data->endPos)\
	    position++;\
    }\
}

static XtTextPosition PSgetLastPos();

static XtTextPosition PSscan (src, pos, sType, dir, count, include)
  XtTextSource src;
  XtTextPosition pos;
  XtTextScanType sType;
  XtTextScanDirection dir;
  int count, include;
{
  int skew, index, ddir,i;
  XtTextPosition  s2Pos, rightmost, leftmost;
  PSContext *ctx = (PSContext *)src->data;
  PieceQueueElement *qp;
    index = pos;
    if(sType == XtstPositions){
        ddir = (dir == XtsdRight) ? 1 : -1;
        if (!include && count > 0)
            count--;
        index = (index + count * ddir);
        index = (index < 0) ? 0 : ((index > ctx->endPos) ? ctx->endPos : index);
        return index;
    }
    if(sType == XtstAll){
        if(dir == XtsdRight)
	    return  PSgetLastPos(src);
	else
	    return 0;
    }
    qp = locate(ctx, &skew, pos);
    s2Pos = pos - skew;
    if(dir == XtsdRight){
	for(i=0; i<count; i++){
	    if(!qp)
		return(ctx->endPos);
	    if (!scan_right(&s2Pos, &skew, &leftmost, &rightmost, &qp, sType)){
		rightmost = leftmost = ctx->endPos;
	    }
	}
        return ((include) ? rightmost : leftmost);
    } else {
	if(!qp)
	    return 0;
	if(!skew){
	    LeftPiece(qp, s2Pos);
		if(!qp)
		    return 0;
	    skew = PieceSize(qp);
	}
	for(i=0; i<count; i++){
	    if(!qp)
		return(0);
	    if (!scan_left(&s2Pos, &skew, &leftmost, &rightmost, &qp, sType)){
		rightmost = leftmost = 0;
	    }
	}
        return ((!include) ? rightmost : leftmost);
    }
}

scan_right(_s2Pos, _skew, _leftmost, _rightmost, _qp, sType)
  XtTextPosition  *_s2Pos;  	/* s2 pos of begin this qp         (bi)  */
  XtTextPosition  *_skew;  	/* offset of actual s2 loc into qp (bi)  */
  XtTextPosition  *_rightmost;  	/* s2 pos of end of target+1       (ret) */  
  XtTextPosition  *_leftmost;   	/* s2 pos of begin this qp         (ret) */
  PieceQueueElement **_qp;        /* piece to start at.  			 */
   XtTextScanType sType;
{
  PieceQueueElement *qp = *_qp;     
  XtTextPosition  s2Pos = *_s2Pos;
  XtTextPosition  leftmost = *_leftmost;
  XtTextPosition  rightmost = *_rightmost;
  XtTextPosition  skew = *_skew;  	
  XtTextPosition  actual, ppos, res;
    ppos = qp->begPos + skew;
    while(qp){
        res = (*(*qp->src)->Scan)(*qp->src, ppos, sType, XtsdRight, 1, 0);
        if(res < qp->endPos){
            break;
        }
	RightPiece(qp, s2Pos);
	if(qp)
	    ppos = qp->begPos;
	else
	    return(0);
    }		
    leftmost = s2Pos + (res - qp->begPos);
    res = (*(*qp->src)->Scan)(*qp->src, ppos, sType, XtsdRight, 1, 1);
    if(res < qp->endPos){
        rightmost = s2Pos + (res - qp->begPos);
	skew = (res - qp->begPos);
    } else {
        RightPiece(qp, s2Pos);
        while(qp){
            res = (*(*qp->src)->Scan)(*qp->src, qp->begPos, sType, XtsdRight, 1, 0);
            if(res != qp->begPos){
                rightmost = s2Pos;
		skew = 0;
		break;
	    }
	    res = (*(*qp->src)->Scan)(*qp->src, qp->begPos, sType, XtsdRight, 1, 1);
            if(res < qp->endPos){
                rightmost = s2Pos + (res - qp->begPos);
	        skew = (res - qp->begPos);
		break;
	    }
	    RightPiece(qp, s2Pos);
	    rightmost = s2Pos;
	}
    }
    *_s2Pos = s2Pos;
    *_skew = skew;
    *_leftmost = leftmost;
    *_rightmost = rightmost;
    *_qp = qp;
    return 1;
}


scan_left(_s2Pos, _skew, _leftmost, _rightmost, _qp, sType)
  XtTextPosition  *_s2Pos;  	/* s2 pos of begin this qp         (bi)  */
  XtTextPosition  *_skew;  	/* offset of actual s2 loc into qp (bi)  */
  XtTextPosition  *_rightmost;  	/* s2 pos of end of target+1       (ret) */  
  XtTextPosition  *_leftmost;   	/* s2 pos of begin this qp         (ret) */
  PieceQueueElement **_qp;        /* piece to start at.  			 */
   XtTextScanType sType;
{
  PieceQueueElement *qp = *_qp;     
  XtTextPosition  s2Pos = *_s2Pos;
  XtTextPosition  leftmost = *_leftmost;
  XtTextPosition  rightmost = *_rightmost;
  XtTextPosition  skew = *_skew;  	
  XtTextPosition  ppos, res;
    leftmost = 0;
    ppos = qp->begPos + skew;
    while(qp){
        res = (*(*qp->src)->Scan)(*qp->src, ppos, sType, XtsdLeft, 1, 0);
        if(res > qp->begPos){
            break;
        }
	LeftPiece(qp, s2Pos);
	if(qp)
	    ppos = qp->endPos;
	else
	    return(0);
    }		
    rightmost = s2Pos + (res - qp->begPos);
    res = (*(*qp->src)->Scan)(*qp->src, ppos, sType, XtsdLeft, 1, 1);
    if(res > qp->begPos){
        leftmost = s2Pos + (res - qp->begPos);
	skew = (res - qp->begPos);
    } else {
        LeftPiece(qp, s2Pos);
        while(qp){
            res = (*(*qp->src)->Scan)(*qp->src, qp->endPos, sType, XtsdLeft, 1, 0);
            if(res != qp->endPos){
                leftmost  = s2Pos + (PieceSize(qp));
		skew = (PieceSize(qp));
		break;
	    }
	    res = (*(*qp->src)->Scan)(*qp->src, qp->endPos, sType, XtsdLeft, 1, 1);
            if(res > qp->begPos){
                leftmost = s2Pos + (res - qp->begPos);
	        skew = (res - qp->begPos);
		break;
	    }
	    leftmost = s2Pos;
	    LeftPiece(qp, s2Pos);
	}
    }
    *_s2Pos = s2Pos;
    *_skew = skew;
    *_leftmost = leftmost;
    *_rightmost = rightmost;
    *_qp = qp;
    return 1;
}

/*
 * 	Read
 */ 

static XtTextPosition PSread (src, pos, text, maxRead)
  XtTextSource src;
  int pos;
  XtTextBlock *text;
  int maxRead;
{
  int pieceSize, skew;
  PSContext *ctx = (PSContext *)src->data;  
  PieceQueueElement *qp;
    if (maxRead == 0){
	text->length = 0;
	return(pos);
    }
    qp = locate(ctx, &skew, pos);
    if (!qp){
	text->length = 0;
	return pos;
    }
    pieceSize = (PieceSize(qp)) - skew;
    if (pieceSize == 0){ 
	text->length = 0;
	return pos;
    }
    text->length = (maxRead > pieceSize) ? pieceSize : maxRead;
    (*(*qp->src)->Read)(*qp->src, (qp->begPos)+skew, text, text->length);
    text->firstPos = pos;
    return (XtTextPosition)(pos + text->length);
}

 
/*
 * 	allocate a change block
 */ 

static ChangeBlock *getChangeBlock()
{
    return((ChangeBlock *)calloc(sizeof(ChangeBlock), 1));
}
 
/* 
 *	Undo a change.
 */
XtTextPosition backoutChange(ctx, change)
  ChangeBlock  *change;
  PSContext *ctx;
{
  int oldEndPos = ctx->endPos;
  ChangeBlock  *newcb;
  PieceQueueElement *where, *qp, *next;
     if(!change)
	return -1;
    newcb = (ChangeBlock*)
	insert(&ctx->CQHead, &ctx->CQTail, getChangeBlock(), ctx->CQTail);
    newcb->where = change->where;
    newcb->deletedPos = change->deletedPos;
    qp = change->addedPtrFirst;
    while(qp){
	next = qp->flink;
	retire(ctx, qp);
	if(qp == change->addedPtrLast)
	    break;
	qp = next;
    }
/* where is the PieceQueue element to put the old stuff back in after */
        where = change->where;
/* remqueue the deleted pieces of the
		 retired list, and queue 'em back in PieceQ*/
    if(change->Lptr){
	ctx->endPos += change->Lfix - change->Lptr->endPos;
	newcb->Lfix = change->Lptr->endPos;
	change->Lptr->endPos = change->Lfix;
	newcb->Lptr = change->Lptr;
    }
    qp = change->retiredQHead;
    while(qp){
	next = qp->flink;
	where = insert(&ctx->PTQHead, &ctx->PTQTail, qp, where);
	if(!newcb->addedPtrFirst)
	    newcb->addedPtrFirst = where;
	ctx->endPos += PieceSize(qp);
        if(qp == change->retiredQTail)
	    break;
	qp = next;
    }
    if(change->Rptr){
	    ctx->endPos += change->Rptr->begPos - change->Rfix;
	    newcb->Rfix = change->Rptr->begPos;
	    change->Rptr->begPos = change->Rfix;
	    newcb->Rptr = change->Rptr;
    }
    if(newcb->addedPtrFirst)
        newcb->addedPtrLast = where;
/*     fixupTextAndSelection(ctx, change->deletedPos, oldEndPos- ctx->endPos); */
    return(change->deletedPos);
}

/*
 *	the Replace operator
 */


static int PSReplace(src, startPos, endPos, text)
  XtTextSource src;
  XtTextPosition startPos, endPos;
  XtTextBlock *text;
{
  int wrote, delLength = endPos - startPos;
  PSContext *ctx = (PSContext *)src->data;  
  PieceQueueElement *here;
  ChangeBlock  *cb;
    ctx->LocatedPiece = 0;
    if(startPos < 0){
        if(!ctx->CQTail)
            return -1;
        ctx->LastUndid = ctx->CQTail;
        return(backoutChange(ctx, ctx->CQTail));
    } else if(endPos < 0){
	if(!ctx->LastUndid)
	    return -1;
	ctx->LastUndid = ctx->LastUndid->blink;
	return(backoutChange(ctx, ctx->LastUndid));
    }
/*
    switch (ctx->editMode) {
        case XttextAppend:
            if (startPos != endPos != data->length)
                return (POSITIONERROR);
            break;
        case XttextRead:
            return (EditError);
        case XttextEdit:
            break;
        default:
            return (EditError);
    }
*/
    if(startPos > ctx->endPos)
        return (EditError);
    ctx->LastUndid = 0;
    if(text->length)
        (*ctx->ao->Replace)(ctx->ao, ctx->aoPos, ctx->aoPos, text);
    else {
	if(!endPos)
	    return 0;
    }
    if((!delLength)&& (startPos==ctx->lastReplaceEndPos) && (ctx->lastAddedPiece)){
	here = ctx->lastAddedPiece;
        if(!((here->endPos == ctx->aoPos) && (here->src == &ctx->ao)))
	    printf("replace optimize panic 1 \n");
	here->endPos += text->length;
    } else {
        ctx->lastReplaceEndPos = -1;
        cb = (ChangeBlock*)insert(&ctx->CQHead, &ctx->CQTail, getChangeBlock(), ctx->CQTail);
        cb->deletedPos = startPos;
        here = deleteRange(ctx, startPos, endPos);
	cb->where = here;
        if(text->length == 0){
	    return (EditDone);
        }
	here = insert(&ctx->PTQHead, &ctx->PTQTail, getQp(), here);
	ctx->lastAddedPiece = here;
        here->begPos = ctx->aoPos;
        here->endPos = ctx->aoPos + text->length;
        here->src = &ctx->ao;
	cb->addedPtrFirst = here;
	if(!cb->addedPtrLast)
	    cb->addedPtrLast = here;
    } 
    ctx->lastReplaceEndPos = startPos + text->length;
    ctx->aoPos += text->length;
    ctx->endPos += text->length;
    return (EditDone);
}

 
static PSsetLastPos(src, lastPos)
  XtTextSourceRec *src;
  XtTextPosition lastPos;
{
    PSContext *ctx = (PSContext *)src->data;
    ctx->endPos = lastPos;
}

static XtTextPosition PSgetLastPos(src)
  XtTextSource src;
{
  PSContext *ctx = (PSContext *)src->data;
    return ctx->endPos;
}

 
static InitPieceTable(ctx)
  PSContext *ctx;
{
  XtTextSourceRec **ro = &(ctx->ro);
  PieceQueueElement *qp;
  XtTextPosition beg, end;
    beg = (*(*ro)->Scan)(*ro, 0, XtstAll, XtsdLeft,  0,0);
    end = (*(*ro)->Scan)(*ro, 0, XtstAll, XtsdRight, 0,0);
    if(beg != end){
        qp = insert(&ctx->PTQHead, &ctx->PTQTail, getQp(), 0);
        qp->begPos = beg;
        qp->endPos = end;
        qp->src = ro;
        ctx->endPos = PieceSize(qp);
    }
}

/*** Public routines ***/
XtTextSource CreatePSource (ro, ao)
  XtTextSourceRec *ro, *ao;
{
  XtTextSourceRec *src;
  PSContext *ctx;
  int     i;
    src = (XtTextSourceRec *) malloc(sizeof(XtTextSourceRec));
    src->Read = PSread;
    src->Replace = PSReplace; 
    src->AddWidget = NULL;
    src->RemoveWidget = NULL;
    src->Scan = PSscan;
    src->GetLastPos =  PSgetLastPos;
    src->GetSelection = NULL;
    src->SetSelection = NULL;
    src->ConvertSelection = NULL;
    ctx = (PSContext *)(calloc(sizeof(PSContext),1));
    ctx->ro = ro;
    ctx->ao = ao;
    ctx->left = ctx->right = 0;
    ctx->source = src;
    InitPieceTable(ctx);
    src->data = (caddr_t)ctx;
    return src;
}
 
PSsetROsource(src, ro)
  XtTextSourceRec *src, *ro;
{
    PSContext *ctx = (PSContext *)src->data;
    ctx->ro = ro;
}
 
 
PSourceDestroy(src)
  XtTextSourceRec *src;
{
PieceQueueElement *qp, *qp_victim;
ChangeBlock *cb, *cb_victim;
    PSContext *ctx = (PSContext *)src->data;
    cb = ctx->CQHead;
    while(cb){
        qp = cb->retiredQHead;
	while(qp){
	    qp_victim = qp;
	    qp = qp->flink;
		free(qp_victim);
	}
	cb_victim = cb;
	cb = cb->flink;
	free(cb_victim);
    }
    qp = ctx->PTQHead;
    while(qp){
	qp_victim = qp;
	qp = qp->flink;
	free(qp_victim);
    }
    free(ctx);
    free(src);
}
 
/*
 *  Application callback is intended for applications that want to
 *  know if the source has ever changed.  
 */
PSsetApplicationCallback(src, callback)
  XtTextSourceRec *src;
  void (*callback)();
{
  PSContext   *ctx = (PSContext *)src->data;
    ctx->applicationCallback = callback;
}


PSchanges(src)
  XtTextSource src;
{
  PSContext   *ctx = (PSContext *)src->data;
  ChangeBlock *cb = ctx->CQHead;
  int i;
    i = 0;
    while(cb){
        i++;
        cb = cb->flink;
    }
    return i;
}


/*
 * 	Force PS to break it's current piece.  
 */
PSbreakInput(src)
  XtTextSourceRec *src;
{
  PSContext   *ctx = (PSContext *)src->data;
	ctx->lastReplaceEndPos = -1;
}
 

int PSUndoLast(src)
XtTextSource src;
{
  PSContext   *data= (PSContext *)src->data;
        if(!data->CQTail)
            return -1;
        data->LastUndid = data->CQTail;
	data->lastReplaceEndPos = -1;
        return(backoutChange(data, data->CQTail));
}

int PSUndoNext(src)
XtTextSource src;
{
  PSContext   *data = (PSContext *)src->data;
	if(!data->LastUndid)
	    return -1;
	data->LastUndid = data->LastUndid->blink;
	data->lastReplaceEndPos = -1;
	return(backoutChange(data, data->LastUndid));
}
