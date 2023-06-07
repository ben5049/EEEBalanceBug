/*
Authors: Advik Chitre
Date created: 03/05/23
*/

//-------------------------------- Imports ----------------------------------------------

import React from 'react';
import Slider from 'react-slick';
import { Link } from 'react-router-dom';
import 'slick-carousel/slick/slick.css';
import 'slick-carousel/slick/slick-theme.css';
import './RoverCarousel.css';
import Rover from './Rover.png';

//-------------------------------- Main -------------------------------------------------

/* 
Rovers carousel component - Carousel to display all rovers taken from the server
*/

const RoverCarousel = ({ rovers }) => {

	//---------------------------- Settings ---------------------------------------------

	const settings = {
		dots: true,
		infinite: false,
		speed: 500,
		slidesToShow: 5,
		slidesToScroll: 1,
		focusOnSelect: true
	};

	//---------------------------- Button Functions: onClick ----------------------------

	/* Select rover button (store seelcted rover's data) */
	const handleRoverClick = (RoverMAC, RoverNickname) => {
		console.log("SELECTED " + RoverMAC)
		localStorage.setItem('MAC', RoverMAC);
		localStorage.setItem('nickname', RoverNickname);
		localStorage.setItem('ConnectedState', 'Start');
	};

	//---------------------------- Button Overlay Functions: ----------------------------

	/* Get overlay text based on connection status */
	const getOverlayText = (connectionStatus) => {
		if (connectionStatus === true){
		return "CONNECT"
		}
		else {
		return "OFFLINE"
		}
	}
	
	/* Get overlay style className based on connection status */
	const getOverlayStyle = (connectionStatus) => {
		if (connectionStatus === true) {
			return 'overlay-green_RoverCarousel';
		}
		else if (connectionStatus === false) {
			return 'overlay-red_RoverCarousel';
		}
		/* Default style for other overlayText values (Error handling) */
		return {};
	};

	//---------------------------- Display ----------------------------------------------

	return (
		<Slider {...settings}>
			{rovers.map((image, index) => (
				<div key={index} className="carousel-item_RoverCarousel">
					{/* Redirects user to rovers page if connected */}
					<Link to={image.connected ? "/connected" : '#' } className="page-link" draggable={false}>
							<button
							className="carousel-button_RoverCarousel"
							onClick={() => handleRoverClick(image.MAC, image.nickname)}
							>
							<img
								src={Rover}
								alt={image.alt}
								style={{
								width: '200px', // Set the desired width
								height: '200px', // Set the desired height
								objectFit: 'cover', // Adjust how the image fits within the container
								}}
							/>
							<p className="image-caption_RoverCarousel">{image.nickname}</p>
							<div className={getOverlayStyle(image.connected)}>
								<p>{getOverlayText(image.connected)}</p>
							</div>
						</button>
					</Link>
				</div>
			))}
		</Slider>
	);
};

export default RoverCarousel;