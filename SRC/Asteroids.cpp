#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include <algorithm>
#include <fstream>


//kllklklkklkllklklkkl/ddfdffdf/dfsdfsdfsfd
// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	// Create a spaceship and add it to the world
	//mGameWorld->AddObject(CreateSpaceship());
	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	//Create the GUI
	CreateGUI();

	LoadHighScores();        //  Load high scores from file
	ShowHighScoreTable();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (!mGameStarted && (key == 's' || key == 'S'))
	{
		mGameStarted = true;

		//hide the menu labels
		if (mStartLabel) mStartLabel->SetVisible(false);
		if (mInstructionsLabel1) mInstructionsLabel1->SetVisible(false);
		if (mInstructionsLabel2) mInstructionsLabel2->SetVisible(false);

		//  hide score table labels
		for (auto& label : mHighScoreLabels)
		{
			label->SetVisible(false);
		}


		//unhide the game labels
		if (mScoreLabel) mScoreLabel->SetVisible(true);
		if (mLivesLabel) mLivesLabel->SetVisible(true);
		
		//error
		mPlayer.SetLives(3); 

		if (mLivesLabel) mLivesLabel->SetText("Lives: 3");
		//

		
		mGameWorld->AddObject(CreateSpaceship());
	}

	if (mGameStarted)
	{
		switch (key)
		{
		case ' ':
			mSpaceship->Shoot();
			break;
		default:
			break;
		}
	}

	if (mIsGameOver)
	{
		// Character input (max 10 chars)
		if (key >= 32 && key <= 126 && mPlayerName.length() < 10)
		{
			mPlayerName += key;
		}

		// Backspace support
		if (key == 8 && !mPlayerName.empty())
		{
			mPlayerName.pop_back();
		}

		// Update label as user types
		if (mNameEntryLabel)
		{
			std::ostringstream msg;
			msg << "Enter Name: " << mPlayerName;
			mNameEntryLabel->SetText(msg.str());
		}

		// Press Enter to save name and score
		if (key == 13 || key == '\r')
		{
			// Get final score from ScoreKeeper
			int score = mScoreKeeper.GetScore();

			// Add to high score list
			mHighScores.push_back({ mPlayerName, score });
			SaveHighScores();

			// Sort high scores by score descending
			std::sort(mHighScores.begin(), mHighScores.end(),
				[](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
					return a.second > b.second;
				});

			// Clean up name entry UI
			mIsGameOver = false;
			mGameOverLabel->SetVisible(false);
			mNameEntryLabel->SetVisible(false);

			// Show top scores on screen
			ShowHighScoreTable();
		}
		return; // block other inputs during game over
	}
}
void Asteroids::ShowHighScoreTable()
{
	float yPos = 0.80f; // start from near the top middle
	int displayCount = std::min((int)mHighScores.size(), 5); // show top 5

	mHighScoreLabels.clear();

	for (int i = 0; i < displayCount; ++i)
	{
		std::ostringstream label;
		label << i + 1 << ". " << mHighScores[i].first << " - " << mHighScores[i].second;

		auto scoreLabel = make_shared<GUILabel>(label.str());
		scoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		scoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);

		mGameDisplay->GetContainer()->AddComponent(
			static_pointer_cast<GUIComponent>(scoreLabel),
			GLVector2f(0.5f, yPos)); // center the label

		mHighScoreLabels.push_back(scoreLabel);

		yPos -= 0.05f; // stack downward
	}
}

void Asteroids::SaveHighScores()
{
	std::ofstream file("highscores.txt");
	if (!file.is_open()) return;

	for (const auto& entry : mHighScores)
	{
		file << entry.first << " " << entry.second << "\n";
	}

	file.close();
}

void Asteroids::LoadHighScores()
{
	std::ifstream file("highscores.txt");
	if (!file.is_open()) return;

	mHighScores.clear();
	std::string name;
	int score;

	while (file >> name >> score)
	{
		mHighScores.push_back({ name, score });
	}

	file.close();

	// Sort scores descending
	std::sort(mHighScores.begin(), mHighScores.end(),
		[](const auto& a, const auto& b) {
			return a.second > b.second;
		});
}




void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
	// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
	// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
	// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	} 
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0) 
		{ 
			SetTimer(500, START_NEXT_LEVEL); 
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mIsGameOver = true;
		mGameOverLabel->SetVisible(true);

		mPlayerName = ""; // Reset name entry

		if (mNameEntryLabel)
		{
			mNameEntryLabel->SetText("Enter Name: ");
			mNameEntryLabel->SetVisible(true);
		}
	}


}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mScoreLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mLivesLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

	// GUI for start
	mStartLabel = make_shared<GUILabel>("Press 'S' to Start the Game");
	mStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mStartLabel->SetVisible(true); 
	shared_ptr<GUIComponent> start_component = static_pointer_cast<GUIComponent>(mStartLabel);
	mGameDisplay->GetContainer()->AddComponent(start_component, GLVector2f(0.5f, 0.5f)); 

	// GUI for instructions
	//1
	mInstructionsLabel1 = make_shared<GUILabel>("Arrow keys to move");
	mInstructionsLabel1->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_RIGHT);
	mInstructionsLabel1->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mInstructionsLabel1->SetVisible(true);
	shared_ptr<GUIComponent> instructions_component1 = static_pointer_cast<GUIComponent>(mInstructionsLabel1);
	mGameDisplay->GetContainer()->AddComponent(instructions_component1, GLVector2f(1.0f, 1.0f)); 
	//2
	mInstructionsLabel2 = make_shared<GUILabel>("Spacebar to shoot");
	mInstructionsLabel2->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_RIGHT);
	mInstructionsLabel2->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mInstructionsLabel2->SetVisible(true);
	shared_ptr<GUIComponent> instructions_component2 = static_pointer_cast<GUIComponent>(mInstructionsLabel2);
	mGameDisplay->GetContainer()->AddComponent(instructions_component2, GLVector2f(1.0f, 0.96f)); 

	// Name entry label for high score input
	mNameEntryLabel = make_shared<GUILabel>("Enter Name: ");
	mNameEntryLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mNameEntryLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mNameEntryLabel->SetVisible(false);

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mNameEntryLabel),
		GLVector2f(0.5f, 0.1f)); // Bottom-center



}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);


}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) 
	{ 
		SetTimer(1000, CREATE_NEW_PLAYER); 
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}




