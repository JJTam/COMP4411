#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include "kumaGlobals.h"
#include "modelerui.h"
#include "kumaModel.h"

extern void kumaInitControls(ModelerControl* controls);

// We need to make a creator function, mostly because of
// nasty API stuff that we'd rather stay away from.
ModelerView* createKumaModel(int x, int y, int w, int h, char *label)
{
	return new KumaModel(x, y, w, h, label);
}

int main()
{
	// Initialize the controls
	// Constructor is ModelerControl(name, minimumvalue, maximumvalue, 
	// stepsize, defaultvalue)
	ModelerControl controls[NUMCONTROLS];
	kumaInitControls(controls);

	Vec3f pcolor(0.2, 0.2, 1.0);
	Vec3f psize(0.05, 0.05, 0.05);
	ParticleSystem *ps = new ParticleSystem(5, 20, psize, pcolor, ParticleType::BALL);
	ModelerApplication::Instance()->SetParticleSystem(ps);
	ModelerApplication::Instance()->Init(&createKumaModel, controls, NUMCONTROLS);
	ModelerApplication::getPUI()->averageMask->value("0.5 0.5");
	return ModelerApplication::Instance()->Run();
}
