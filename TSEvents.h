/*
 * Touch event handler template.
 * Subclass a template instantiated for a touchscreen
 * with an API that does the right things, and pass an
 * touchscreen instance to the constructor.
 * Your subtype should define any methods you need from:
 * touch, repeat, release, motion, dragTo.
 *
 * On a touch, you can call dragCapture with any non-zero value.
 * That sets a drag capture, so you will get dragTo events
 * instead of repeat()s as the touch moves. The capture gets
 * removed when the touch stops.
 */

/*
 * This is calibration data for the raw touch data to the screen coordinates.
 * The bare chip sends values in the range 0..4095. These set the useful range.
 * You will need to adjust these values to set the X and Y ranges to 0..320, 240
 */
#define TS_MINX		450
#define TS_MAXX		3800
#define	TS_WIDTH	320
#define TS_MINY		130
#define TS_MAXY		4000
#define	TS_HEIGHT	240

#define TS_AUTOREPEAT           250     // Auto-repeat time in ms
#define TS_MOTION_THRESHOLD     10      // pixels; we don't use motion

template <typename TS>
class TSEvents
{			// You must subclass this to define the pure virtual functions
public:
	int		begin() { return ts.begin(); }
	void		detect();		// Process an event if there's one
	virtual void	touch(int x, int y) = 0;
	virtual void	repeat(int x, int y) = 0;
	virtual void	release(int x, int y) {}
	virtual void	motion(int x, int y)
			{ if (capture_id) dragTo(capture_id, x, y); }
	virtual void	dragTo(int capture_id, int x, int y)
			{}

	void 		dragCapture(int id)
			{ capture_id = id; }

protected:
	TSEvents(TS _ts)
	: ts(_ts)
	, touching(false)
	, repeat_from(0)
	, last_x(0)
	, last_y(0)
	, capture_id(0)
	{}
	~TSEvents() {}

	TS		ts;
	bool		touching;	// Were we touching?
	unsigned long	repeat_from;	// millis() as at last touch or auto-repeat
	int  		last_x;
	int		last_y;
	int     	capture_id;	// If non-zero, we're dragging. The value carries context.

	void		showXY(const char* why, int x, int y)
			{
				Serial.print(why);
				Serial.print("(");
				Serial.print(x);
				Serial.print(',');
				Serial.print(y);
				Serial.println(")");
			}

	inline int iabs(int i) { return i < 0 ? -i : i; }
};

template <typename TS>
void TSEvents<TS>::detect()
{
  int         x, y;
  if (ts.touched()) {
    TS_Point p = ts.getPoint();

    // Scale from ~0->4000 to TS_WIDTH, TS_HEIGHT using the calibration #'s
    x = map(p.x, TS_MINX, TS_MAXX, 0, TS_WIDTH);
    y = map(p.y, TS_MINY, TS_MAXY, 0, TS_HEIGHT);

    if (!touching) {            // Touch just started
      touching = true;
      touch(x, y);
      repeat_from = millis();
    } else if (millis()-TS_AUTOREPEAT > repeat_from) { // auto-repeat
      if (!capture_id)		// Don't repeat while we have a drag capture
        repeat(x, y);
      repeat_from = millis();
    } else {                    // Not yet time to auto-repeat
      if (iabs(x-last_x) + iabs(y-last_y) < TS_MOTION_THRESHOLD)
        return;                 // Not enough motion to matter
      motion(x, y);
    }
    last_x = x;
    last_y = y;
  } else {
    if (!touching)
      return;
    release(last_x, last_y);
    dragCapture(0);
    touching = false;
  }
}
