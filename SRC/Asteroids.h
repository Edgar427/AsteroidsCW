#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include <string>
#include <vector>
#include <utility>


class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world);
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;
	shared_ptr<GUILabel> mStartLabel;
	shared_ptr<GUILabel> mInstructionsLabel1;
	shared_ptr<GUILabel> mInstructionsLabel2;
	shared_ptr<GUILabel> mInstructionsLabel3;
	shared_ptr<GUILabel> mReturnToMenuLabel;
	shared_ptr<GUILabel> mInvulnerableLabel; 
	shared_ptr<GUILabel> mExtraLifeLabel;
	shared_ptr<GUILabel> mInvulnerabilityPowerUpLabel;
	shared_ptr<GUILabel> mInstructionsLabel4;
	shared_ptr<GUILabel> mInstructionsLabel5;

	bool mGameStarted = false;
	
	bool mIsHighScoreDisplayed = false;


	// power up toggles
	bool mEnableExtraLifePowerUp = true;
	bool mEnableInvulnerabilityPowerUp = true;




	// High score logic
	bool mIsGameOver = false;
	std::string mPlayerName;
	std::vector<std::pair<std::string, int>> mHighScores;

	// Label to show name entry
	shared_ptr<GUILabel> mNameEntryLabel;
	std::vector<std::shared_ptr<GUILabel>> mHighScoreLabels; //  store score labels

	
	void LoadHighScores();
	void SaveHighScores();
	void ShowHighScoreTable();
	void ResetToMainMenu(); 
	void CreateExtraLifePowerUp();
	void CreateInvulnerabilityPowerUp();
	void RemoveAllPowerUps();
	


	const static uint SPAWN_EXTRA_LIFE_POWERUP = 4;
	const static uint SPAWN_INVULNERABILITY_POWERUP = 5;  





	uint mLevel;
	uint mAsteroidCount;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();
	
	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;
};

#endif