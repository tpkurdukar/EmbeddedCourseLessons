/*.$file${.::ship.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: game.qm
* File:  ${.::ship.c}
*
* This code has been generated by QM 5.1.0 <www.state-machine.com/qm/>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*/
/*.$endhead${.::ship.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qpc.h"
#include "bsp.h"
#include "game.h"

/* Q_DEFINE_THIS_FILE */

#define SHIP_WIDTH  5
#define SHIP_HEIGHT 3

/* encapsulated delcaration of the Ship active object ----------------------*/
/*.$declare${AOs::Ship} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::Ship} ............................................................*/
typedef struct {
/* protected: */
    QActive super;

/* private: */
    uint8_t x;
    uint16_t y;
    uint8_t exp_ctr;
    uint16_t score;
} Ship;

/* protected: */
static QState Ship_initial(Ship * const me, void const * const par);
static QState Ship_active(Ship * const me, QEvt const * const e);
static QState Ship_parked(Ship * const me, QEvt const * const e);
static QState Ship_flying(Ship * const me, QEvt const * const e);
static QState Ship_exploding(Ship * const me, QEvt const * const e);
/*.$enddecl${AOs::Ship} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* local objects -----------------------------------------------------------*/
static Ship l_ship; /* the sole instance of the Ship active object */

/* Public-scope objects ----------------------------------------------------*/
/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 680U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.8.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${AOs::AO_Ship} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

/* opaque AO pointer */
/*.${AOs::AO_Ship} .........................................................*/
QActive * const AO_Ship = &l_ship.super;
/*.$enddef${AOs::AO_Ship} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* Active object definition ------------------------------------------------*/
/*.$define${AOs::Ship_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::Ship_ctor} .......................................................*/
void Ship_ctor(void) {
    Ship *me = &l_ship;
    QActive_ctor(&me->super, Q_STATE_CAST(&Ship_initial));
    me->x = GAME_SHIP_X;
    me->y = (GAME_SHIP_Y << 2);
}
/*.$enddef${AOs::Ship_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${AOs::Ship} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::Ship} ............................................................*/
/*.${AOs::Ship::SM} ........................................................*/
static QState Ship_initial(Ship * const me, void const * const par) {
    /*.${AOs::Ship::SM::initial} */
    (void)par; /* usused parameter */

    QActive_subscribe(&me->super, TIME_TICK_SIG);
    QActive_subscribe(&me->super, PLAYER_TRIGGER_SIG);

    /* local signals... */
    //QS_SIG_DICTIONARY(PLAYER_SHIP_MOVE_SIG, me);
    QS_SIG_DICTIONARY(TAKE_OFF_SIG,         me);
    QS_SIG_DICTIONARY(HIT_WALL_SIG,         me);
    QS_SIG_DICTIONARY(HIT_MINE_SIG,         me);
    QS_SIG_DICTIONARY(DESTROYED_MINE_SIG,   me);

    QS_FUN_DICTIONARY(&Ship_active);
    QS_FUN_DICTIONARY(&Ship_parked);
    QS_FUN_DICTIONARY(&Ship_flying);
    QS_FUN_DICTIONARY(&Ship_exploding);

    return Q_TRAN(&Ship_active);
}
/*.${AOs::Ship::SM::active} ................................................*/
static QState Ship_active(Ship * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::Ship::SM::active::initial} */
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&Ship_parked);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*.${AOs::Ship::SM::active::parked} ........................................*/
static QState Ship_parked(Ship * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::Ship::SM::active::parked::TAKE_OFF} */
        case TAKE_OFF_SIG: {
            status_ = Q_TRAN(&Ship_flying);
            break;
        }
        default: {
            status_ = Q_SUPER(&Ship_active);
            break;
        }
    }
    return status_;
}
/*.${AOs::Ship::SM::active::flying} ........................................*/
static QState Ship_flying(Ship * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::Ship::SM::active::flying} */
        case Q_ENTRY_SIG: {
            ScoreEvt *sev;
            me->score = 0U; /* reset the score */
            sev = Q_NEW(ScoreEvt, SCORE_SIG);
            sev->score = me->score;
            QACTIVE_POST(AO_Tunnel, (QEvt *)sev, me);

            /* lauch the ship from the initial position */
            me->x = GAME_SHIP_X;
            me->y = (GAME_SHIP_Y << 2);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Ship::SM::active::flying::TIME_TICK} */
        case TIME_TICK_SIG: {
            ObjectImageEvt *oie;

            if (BSP_isThrottle()) {
                if (me->y > 0) {
                    me->y -= 1U;
                }
            }
            else {
                if (me->y < (GAME_TUNNEL_HEIGHT << 2)) {
                    me->y += 1U;
                }
            }

            /* tell the Tunnel to draw the Ship and test for hits */
            oie = Q_NEW(ObjectImageEvt, SHIP_IMG_SIG);
            oie->x   = me->x;
            oie->y   = (uint8_t)(me->y >> 2);
            oie->bmp = SHIP_BMP;
            QACTIVE_POST(AO_Tunnel, (QEvt *)oie, me);

            ++me->score; /* increment the score for surviving another tick */

            if ((me->score % 10U) == 0U) { /* is the score "round"? */
                ScoreEvt *sev = Q_NEW(ScoreEvt, SCORE_SIG);
                sev->score = me->score;
                QACTIVE_POST(AO_Tunnel, (QEvt *)sev, me);
            }
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Ship::SM::active::flying::PLAYER_TRIGGER} */
        case PLAYER_TRIGGER_SIG: {
            ObjectPosEvt *ope = Q_NEW(ObjectPosEvt, MISSILE_FIRE_SIG);
            ope->x = me->x;
            ope->y = (me->y >> 2) + SHIP_HEIGHT - 1U;
            QACTIVE_POST(AO_Missile, (QEvt *)ope, me);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Ship::SM::active::flying::DESTROYED_MINE} */
        case DESTROYED_MINE_SIG: {
            me->score += Q_EVT_CAST(ScoreEvt)->score;
            /* the score will be sent to the Tunnel by the next TIME_TICK */
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Ship::SM::active::flying::HIT_WALL} */
        case HIT_WALL_SIG: {
            status_ = Q_TRAN(&Ship_exploding);
            break;
        }
        /*.${AOs::Ship::SM::active::flying::HIT_MINE} */
        case HIT_MINE_SIG: {
            status_ = Q_TRAN(&Ship_exploding);
            break;
        }
        default: {
            status_ = Q_SUPER(&Ship_active);
            break;
        }
    }
    return status_;
}
/*.${AOs::Ship::SM::active::exploding} .....................................*/
static QState Ship_exploding(Ship * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::Ship::SM::active::exploding} */
        case Q_ENTRY_SIG: {
            me->exp_ctr = 0U;
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Ship::SM::active::exploding::TIME_TICK} */
        case TIME_TICK_SIG: {
            /*.${AOs::Ship::SM::active::exploding::TIME_TICK::[me->exp_ctr<15U]} */
            if (me->exp_ctr < 15U) {
                ObjectImageEvt *oie;
                ++me->exp_ctr;
                /* tell the Tunnel to draw the current stage of Explosion */
                oie = Q_NEW(ObjectImageEvt, EXPLOSION_SIG);
                oie->bmp = EXPLOSION0_BMP + (me->exp_ctr >> 2);
                oie->x   = me->x; /* x of explosion */
                oie->y   = (int8_t)((int)(me->y >> 2) - 4U + SHIP_HEIGHT);
                QACTIVE_POST(AO_Tunnel, (QEvt *)oie, me);
                status_ = Q_HANDLED();
            }
            /*.${AOs::Ship::SM::active::exploding::TIME_TICK::[else]} */
            else {
                ScoreEvt *gameOver = Q_NEW(ScoreEvt, GAME_OVER_SIG);
                gameOver->score = me->score;
                QACTIVE_POST(AO_Tunnel, (QEvt *)gameOver, me);
                status_ = Q_TRAN(&Ship_parked);
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Ship_active);
            break;
        }
    }
    return status_;
}
/*.$enddef${AOs::Ship} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
