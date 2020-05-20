#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "include/ocv_versioninfo.h"
#include "opencv2/features2d.hpp"
#include "blackjack_fwd.h"
#include <time.h>

unsigned cardScore(int card_value){
	/***
		Return the score associated to the card with value card_value
	***/
	return (card_value == 1)? 11: std::min(10, card_value);
}

bool filterCorners(std::vector<cv::Point> contour_poly){
	/***
		Return TRUE if the contour has exactly 4 corners, ELSE otherwise
	***/
	return contour_poly.size()==4;
}

bool filterByArea(std::vector<cv::Point> contour_poly, unsigned min_thr = MIN_AREA_THR, unsigned max_thr = MAX_AREA_THR){
	/***
		Return TRUE if the contour has area between MIN_AREA_THR and MAX_AREA_THR, ELSE otherwise
	***/
	cv::Rect boundRect;/*** bounding rectangle is created around the contour **/
	boundRect = cv::boundingRect(cv::Mat(contour_poly));
	
	double area_rect = boundRect.height * boundRect.width;
	return area_rect > min_thr && area_rect < max_thr;
}

void filterContours(std::vector<std::vector<cv::Point>> contours, std::vector<GraphicalCard>& gcards, size_t bound_thr = 400){
	/***
		Return into the vector gcards a list of candidate shapes, each of which represent a card on the board
	***/
	std::vector<std::vector<cv::Point> > contours_poly(contours.size());

	for (unsigned i = 0; i < contours.size(); i++){
		size_t contour_size = contours[i].size();
		if(contour_size > bound_thr){
			/***
				- arcLength with parameter true computes a closed contour perimeter for the contour given in input
				- approxPolyDP approximates a polygonal curve with perimeter precision.
			***/
			double perimeter = cv::arcLength(contours[i],true);
			cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 0.01*perimeter, true); // Ramer–Douglas–Peucker algorithm

			// Filter contours: a card has 4 corners and its area belongs to a certain range
			if(filterCorners(contours_poly[i]) && filterByArea(contours_poly[i])){
				cv::drawContours(scene.debug, contours, i, cv::Scalar(80, 2, 255), 3);  // for debug 
				cv::drawContours(scene.debug, contours_poly, i, cv::Scalar(120, 50, 155), 3);  // for debug 
				gcards.push_back(GraphicalCard(contours_poly[i],contours[i]));
			}
		}
	}
}

bool isHorizontal(cv::Point topLeft, cv::Point topRight, cv::Point bottomLeft, cv::Point bottomRight){
	/***
		Return TRUE if the 4 corners describe an horizontal shape, ELSE otherwise.
		Compute height and width using Euclidean distance and compare them.
	***/
	double height = std::sqrt(std::pow(bottomLeft.y - topLeft.y,2) + std::pow(bottomRight.y - topRight.y,2));
	double width = std::sqrt(std::pow(topRight.x - topLeft.x,2) + std::pow(bottomRight.x - bottomLeft.x,2));

	return width > height;
}

cv::Point computeCenter(const std::vector<cv::Point>& corners){
	/***
		Return the center of the shape described by corners
	***/
	cv::Point result;
	
	for (size_t i = 0; i < corners.size(); ++i)
		result += corners[i];

	result *= (1.0 / corners.size());
	return result;
}

void sortCorners(std::vector<cv::Point>& corners){
	/***
		Given a vector of 4 corners of a card sort them in order to have the right 
		combination of topLeft, topRight, bottomLeft and bottomRight corners
	***/
	std::vector<cv::Point> top, bottom;
	cv::Point center = computeCenter(corners);

	for (size_t i = 0; i < corners.size(); ++i){
		if (corners[i].y < center.y)
			top.push_back(corners[i]);
		else
			bottom.push_back(corners[i]);
	}
	corners.clear();

	//if (top.size() == 2 && bottom.size() == 2){
	cv::Point topLeft = top[0].x > top[1].x ? top[1] : top[0];
	cv::Point topRight = top[0].x > top[1].x ? top[0] : top[1];
	cv::Point bottomLeft = bottom[0].x > bottom[1].x ? bottom[1] : bottom[0];
	cv::Point bottomRight = bottom[0].x > bottom[1].x ? bottom[0] : bottom[1];

	if(isHorizontal(topLeft, topRight, bottomLeft, bottomRight)){
		corners.push_back(topRight);
		corners.push_back(bottomRight);
		corners.push_back(bottomLeft);
		corners.push_back(topLeft);
	}else{
		corners.push_back(topLeft);
		corners.push_back(topRight);
		corners.push_back(bottomRight);
		corners.push_back(bottomLeft);
	}
	//}
}

unsigned evalColor(cv::Mat warped_card){
	/***
		Return the card color RED = 2, BLACK = 0
		The color is obtained considering only the top left angle, where the seed symbol is located.
	***/
	cv::resize(warped_card, warped_card, cv::Size(100,100));
	cv::Rect crop(5, 5, 20 ,10);
	cv::Mat seed(warped_card, crop);
	cv::Size size_seed(50,70);

	cv::resize(seed, seed, size_seed);
	cv::Mat3b hsv;
	cv::cvtColor(seed, hsv, cv::COLOR_BGR2HSV);
	cv::Mat1b mask1;
	// check where the image contains a red range
	cv::inRange(hsv, cv::Scalar(0, 70, 50), cv::Scalar(10, 255, 255), mask1);
	//cv::imshow("seed", seed);
	double min, max;
	cv::minMaxLoc(mask1, &min, &max);
	return (max == 255)? RED: BLACK;//median
}

void warpCard(GraphicalCard& gcard){
	/***
		Given a graphical card on the table find the perspective transformation and warp it into the warped_card attribute of GraphicalCard
	***/
	cv::Size out_size(CARD_WIDTH, CARD_HEIGHT);

	std::vector<cv::Point2f> dst_pts;
	dst_pts.push_back(cv::Point2f(0,0)); dst_pts.push_back(cv::Point2f(out_size.width,0) );
	dst_pts.push_back(cv::Point2f(out_size.width,out_size.height) ); dst_pts.push_back(cv::Point2f(0,out_size.height) );

	// sort corners in order to have an unique way to assign corners to the destination image
	std::vector<cv::Point2f> src_pts;
	sortCorners(gcard.corners);

	src_pts.push_back(gcard.corners[0]); src_pts.push_back(gcard.corners[1]);
	src_pts.push_back(gcard.corners[2]); src_pts.push_back(gcard.corners[3]);

	// findHomography finds a perspective transformation between two planes. it returns the transformation matrix from one plane to the other
	cv::Mat H = cv::findHomography(src_pts, dst_pts, cv::RANSAC);
	/***
		The function warpPerspective transforms the source image using the specified matrix, in our case we use H
		which is the transformation matrix obtained from findHomography.***/
	cv::warpPerspective(scene.InFrame, gcard.warped_card, H, out_size);
	gcard.color = evalColor(gcard.warped_card);
	cv::imshow("warped card", gcard.warped_card);
}

void drawDetectedCard(Card card){
	/***
		Draw detected card on the board and its matching score.***/
	cv::Point center = computeCenter(card.gcard.corners);
	cv::putText(scene.debug, std::to_string(card.value) + " "+ card.seed , cv::Point(center.x - 100, center.y), cv::FONT_HERSHEY_PLAIN, 2.5, cv::Scalar(0,255,0), 4.0);
	cv::putText(scene.debug, std::to_string(card.score), cv::Point(center.x - 70, center.y+60), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(255,100,50), 3.0);
}


Card matchCard(GraphicalCard gcard){
	/***
		Match the card with the template in order to get value and seed information.
		Froma  GraphicalCard is returned a Card, which will contain value, seed and matching score.***/
	cv::Mat card = gcard.warped_card;
	/***
		In order to reduce the neeed computation get the color of the card and use it to decide if the card is in the red or black model
		in this way the matching function is faster since only half of the cards are checked on the model.***/
	cv::Mat model = (gcard.color == RED)?red_model:black_model;
	
	cv::cvtColor(card,card, cv::COLOR_RGB2GRAY);

	cv::Mat corr;
	cv::matchTemplate(card, model, corr, cv::TM_CCORR_NORMED);
	
	double max, min;
	cv::Point minP, maxP;
	cv::minMaxLoc(corr, &min, &max, &minP, &maxP, cv::Mat());
	
	/***
		If the card is rotated the match could be not perfect, for this reason it is necessary to evaluate also the rotated version of the 
		card and consider the best match as final result. This problem can be important with cards like 7, since the match with its rotated
		version provides a small score or not the best one.***/
	cv::Mat corrR;
	cv::rotate(card, card, cv::ROTATE_180);
	cv::matchTemplate(card, model, corrR, cv::TM_CCORR_NORMED);
	
	double maxR, minR;
	cv::Point minPR, maxPR;
	cv::minMaxLoc(corrR, &minR, &maxR, &minPR, &maxPR, cv::Mat());/*** Take the best match between the two matching results ***/
	cv::Point maxPoint = (max > maxR)? maxP: maxPR;

	unsigned value = maxPoint.y/CARD_HEIGHT+1;
	unsigned seed_idx = maxPoint.x/CARD_WIDTH + gcard.color;

	return Card (gcard, value, seed_names[seed_idx], seed_idx, max);
}

double evalPrLosing(int res_score, unsigned residuals){
	/*** 
		Return the probability of loosing, score is greater then 21, with the next draw.
	 ***/
	if(res_score <= 1) return 1;
	if(res_score > 11) return 0;
	double pr_loss = 0;
	
	for(unsigned i = 0; i < 4; ++i)
		if(scene.cards_status[i] == IN_DECK)
			pr_loss+= 1;

	for(unsigned i = res_score*4; i < 52; ++i)
		if(scene.cards_status[i] == IN_DECK)
			pr_loss+= 1;

	return pr_loss/residuals;
}

double evalPrWin(int res_score, unsigned residuals){
	/*** 
		Return the probability of winning, score is exactly 21, with the next draw.
	 ***/
	double pr_win = 0;
	unsigned nr_idx = (res_score-1)*4;

	if(res_score <= 1 || res_score > 11) return 0;

	for(unsigned i = nr_idx; i < nr_idx+4; ++i)
		if(scene.cards_status[i] == IN_DECK)
			pr_win+= 1;

	return pr_win/residuals;
}

std::vector<double> evalProbabilities(int res_score){
	/*** 
		Return a vector that contains the 3 probabilities: pr of winning, pr lower 21 and pr of loosing
	 ***/
	std::vector<double> prob; /*pr_loss, p_l21, pr_win*/

	unsigned residuals = 0; 
	for (auto card_status : scene.cards_status)
		if(card_status == IN_DECK)
			residuals += 1;

	double pr_loss = evalPrLosing(res_score, residuals);
	double pr_win = evalPrWin(res_score, residuals);
	//pr_l21 is the probability that with the next draw the score is lower than 21
	double pr_l21 = std::max(1 - pr_loss - pr_win,0.0);

	prob.push_back(pr_loss);
	prob.push_back(pr_l21);
	prob.push_back(pr_win);
	return prob;
}

void drawCardsStatusBoard(cv::Mat& status){
	/***
		Draw GUI for cards status
	***/
	cv::Mat cards_status(970, 500, CV_8UC3, cv::Scalar(30,30,30));
	cv::Point pos(30,30);
	unsigned split_layout = scene.cards_status.size()/2-1;
	for(unsigned i = 0; i < scene.cards_status.size(); ++i){
		std::string card = std::to_string(i/4 +1) +" "+ seed_names[i%4]; 
		cv::putText(cards_status, card, pos, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2,  status_color[scene.cards_status[i]], 1.5);

		pos+= cv::Point(0, 32);
		if(i == split_layout)
			pos = cv::Point(260,30);
	}
	cv::rectangle(cards_status, cv::Point2f(30, 860), cv::Point2f(50, 880), IN_DECK_COLOR, 3);
	cv::rectangle(cards_status, cv::Point2f(30, 900), cv::Point2f(50, 920), IN_TABLE_COLOR, 3);
	cv::rectangle(cards_status, cv::Point2f(30, 940), cv::Point2f(50, 960), REMOVED_COLOR, 3);

	cv::putText(cards_status, "IN DECK",  cv::Point2f(80, 880), cv::FONT_HERSHEY_PLAIN, 1.7, IN_DECK_COLOR, 2.5);
	cv::putText(cards_status, "IN TABLE",  cv::Point2f(80, 920), cv::FONT_HERSHEY_PLAIN, 1.7, IN_TABLE_COLOR, 2.5);
	cv::putText(cards_status, "REMOVED",  cv::Point2f(80, 960), cv::FONT_HERSHEY_PLAIN, 1.7, REMOVED_COLOR, 2.5);

	cv::putText(cards_status, "@Cinofix", cv::Point(325, 960), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(211, 211, 211), 2.0);

	status = cards_status;
}

void drawStatusBoard(cv::Mat& status_board){
	/***
		Draw game status on status_board. Status is composed by:
			- Cards on table
			- Score and residual
			- Probabilities
			- Cards status: IN DECK, IN TABLE, REMOVED
	***/
	initStatusColors();
	cv::Mat status(250, 1280, CV_8UC3, cv::Scalar(30,30,30));
	cv::Point pos(650, 50);
	cv::putText(status, "Extracted Cards: ", cv::Point(50, 50), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(0, 0, 255), 2.0);

	unsigned score = 0;
	for(Card card: scene.cards){
		cv::putText(status, std::to_string(card.value), pos, cv::FONT_HERSHEY_PLAIN, 3,  cv::Scalar(40, 150, 255), 2.5);
		pos = pos + cv::Point(100,0);
		score+= cardScore(card.value);

		scene.cards_status[(card.value-1)*4 + card.seed_idx] = IN_TABLE;
	}

	cv::putText(status, "Score: ", cv::Point(50, 120), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(0, 0, 255), 2.0);
	cv::putText(status, std::to_string(score), cv::Point(250, 120), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(255, 150, 0), 2.0);

	int residual_score = 21 - score;
	std::vector<double> prob = evalProbabilities(residual_score);

	cv::putText(status, "Residual: ", cv::Point(650, 120), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(0, 0, 255), 2.0);
	cv::putText(status, std::to_string(residual_score), cv::Point(900, 120), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(255, 150, 0), 2.0);

	cv::putText(status, "Pr. Win: ", cv::Point(50, 210), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(50, 250, 100), 3.0);
	cv::putText(status, "Pr. Lw: ", cv::Point(465, 210), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(40, 150, 255), 3.0);
	cv::putText(status, "Pr. Lose: ", cv::Point(850, 210), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(70, 50, 250), 3.0);
	
	cv::putText(status, cv::format("%1.3f", prob[2]), cv::Point(270, 210), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(40, 175, 30), 2.0);	
	cv::putText(status, cv::format("%1.3f", prob[1]), cv::Point(665, 210), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(120, 200, 255), 2.0);
	cv::putText(status, cv::format("%1.3f", prob[0]), cv::Point(1100, 210), cv::FONT_HERSHEY_PLAIN, 3.0,  cv::Scalar(30, 20, 150), 2.0);
	
	status_board = status;
}

void updateRemoved(){
	/***
		Set previous matched cards to REMOVED
	***/
	for(Card card: scene.cards)
		scene.cards_status[(card.value-1)*4 + card.seed_idx] = REMOVED;
}

int main( int argc, char* argv[]){

	init_model();

	cv::VideoCapture testVideo("test/test_video_2.mp4");
	bool play_video = true;

	unsigned frames = 0;
	time_t start, end;
	time(&start);
	while(1){
		if(play_video){
			cv::Mat frame;
			testVideo >> frame;
			scene.InFrame = frame;
			scene.debug = frame.clone(); 

			const clock_t begin_time = clock();			
			if (frame.empty())
				break;
			frames += 1;

			cv::Mat bwFrame;
			cv::cvtColor(frame,bwFrame, cv::COLOR_RGB2GRAY);
			cv::threshold(bwFrame,scene.IbwFrame, 128, 255, cv::THRESH_BINARY);

			/*** Find contours on the frame ***/
			std::vector<std::vector<cv::Point> > contours;
			cv::findContours(scene.IbwFrame, contours, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

			/*** Pick only cards contours ***/
			std::vector<GraphicalCard> gcards;
			filterContours(contours, gcards);

			scene.ncards = gcards.size();
			cv::putText(scene.debug, "#Cards: "+std::to_string(scene.ncards), cv::Point(60, 650), cv::FONT_HERSHEY_PLAIN, 4.0, cv::Scalar(0,255,0), 2.0);
			scene.cards.clear();
			/*** For each graphical card detected warp, match and draw it on the board ***/
			for(int i = 0; i < scene.ncards; i++){
				warpCard(gcards[i]);
				Card detected_card = matchCard(gcards[i]);
				scene.cards.push_back(detected_card);
				drawDetectedCard(detected_card);
			}
			
			cv::Mat status;
			drawStatusBoard(status);
			cv::vconcat(scene.debug, status, status);

			cv::Mat cards_status;
			drawCardsStatusBoard(cards_status);
			cv::hconcat(status, cards_status, status);

			updateRemoved();
	  		cv::imshow("BlackJack Counter", status);

  			time(&end);
			double seconds = difftime (end, start);
			double fps  = 1 / seconds;
			std::cout << "\n Frame execution time: "<<float( clock () - begin_time ) /  CLOCKS_PER_SEC;
		}
		char key = (char)cv::waitKey(1);
		if(key=='p') /*** Pause video ***/
			play_video = !play_video;
		if(key=='q')
			break;
	}
	time(&end);
    double seconds = difftime (end, start);
    double fps  = frames / seconds;
    std::cout << "\nFPS: "<< fps<<"\n";
	testVideo.release();
	return 0;
}