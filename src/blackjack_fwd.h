#ifndef _BLACKJACK_FWD_H_
#define _BLACKJACK_FWD_H_

/*** Constants used to define min area and maximum area used for contours filtering***/
const unsigned MIN_AREA_THR = 20000;
const unsigned MAX_AREA_THR = 100000;

/*** 
	Card size! The greater they are the slower the counter software is but its robustness is increased
	It is sudjested to preserve proportions provided by the model.
***/
const unsigned CARD_WIDTH = 73;
const unsigned CARD_HEIGHT = 103;

/*** Card Color! RED is equal to 2 since seed names of red cards start from position 2 in seed_names***/
const unsigned RED = 2;
const unsigned BLACK = 0;

/*** Card status **/
const unsigned IN_DECK = 0;
const unsigned IN_TABLE = 1;
const unsigned REMOVED = 2;

/*** Color for each status ***/
const cv::Scalar IN_DECK_COLOR(255, 175, 30);
const cv::Scalar IN_TABLE_COLOR(90, 127, 253);
const cv::Scalar REMOVED_COLOR(154,62,135);

std::vector<cv::Scalar> status_color;
void initStatusColors(){
	/*** Init status colors ***/
	status_color.push_back(IN_DECK_COLOR); // IN_DECK
	status_color.push_back(cv::Scalar(90, 127, 253)); // IN_TABLE
	status_color.push_back(cv::Scalar(154,62,135)); // REMOVED
}

struct GraphicalCard{
	/*** 
		Graphical rapresentation for each card on the board. A graphical card is composed by:
			- contours
			- 4 corners
			- warp trasformation
			- detected color of the card
	***/
	std::vector<cv::Point> contours;
	std::vector<cv::Point> corners;
	cv::Mat warped_card;
	unsigned color;

	GraphicalCard(std::vector<cv::Point> corners_, std::vector<cv::Point> contours_): corners(corners_), contours(contours_){} 
};

struct Card{
	/*** 
		A Card is the complete type that contains all the needed information for a card. It contains its graphical rapresentation, value and seed detected
	***/
	GraphicalCard gcard;
	unsigned value;
	std::string seed;
	unsigned seed_idx;
	double score;

	Card(GraphicalCard gcard_, unsigned v_, std::string seed_, unsigned idx_seed_, double score_): gcard(gcard_), value(v_), seed_idx(idx_seed_), seed(seed_), score(score_){} 
};

struct Board{
	/***
		Board contains all the information about the game and GUI
	***/
	cv::Mat InFrame;
	cv::Mat debug;
	cv::Mat IbwFrame;
	unsigned ncards;
	std::vector<Card> cards;
	std::vector<unsigned> cards_status;

	Board(): cards_status(52, IN_DECK){}
};


Board scene;

std::vector<std::string> seed_names;
void init_names(){
	/*** Init vector of seed names ***/
	seed_names.push_back("spades");
	seed_names.push_back("clubs");
	seed_names.push_back("diamond");
	seed_names.push_back("hearts");
}



cv::Mat red_model;
cv::Mat black_model;
void init_model(){
	/*** Init model for matching ***/
	init_names();

	red_model = cv::imread("model/red_cards.jpg");
	black_model = cv::imread("model/black_cards.jpg");
	
	cv::Size scale(red_model.cols/6, red_model.rows/6);
	cv::resize(red_model, red_model, scale);
	cv::resize(black_model, black_model, scale);
	
	cv::cvtColor(black_model,black_model, cv::COLOR_RGB2GRAY);
	cv::cvtColor(red_model,red_model, cv::COLOR_RGB2GRAY);
}

#endif //_BLACKJACK_FWD_H_