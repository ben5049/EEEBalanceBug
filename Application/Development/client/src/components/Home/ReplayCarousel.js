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
import './ReplayCarousel.css';

//-------------------------------- Main -------------------------------------------------

/* 
Replay carousel component - Carousel to display all replays taken from the server
*/

const ReplayCarousel = ({ replays }) => {

  	//---------------------------- Settings ---------------------------------------------

    const settings = {
        dots: true,
        infinite: false,
        speed: 500,
        slidesToShow: 6,
        slidesToScroll: 1,
        focusOnSelect: true
    };
    
	//---------------------------- Button Functions: onClick ----------------------------

	/* Select replay button */
    const handleReplayClick = (ReplayID) => {
        console.log("SELECTED " + ReplayID)
        localStorage.setItem('ReplayID', ReplayID);
    };

	//---------------------------- Display ----------------------------------------------

	return (
		<Slider {...settings}>
			{replays.map((image, index) => (
				<div key={index} className="carousel-item">
					{/* Redirects user to replay page */}
					<Link to="/replay" className="page-link" draggable={false}>
						<button
						className="carousel-button"
						onClick={() => handleReplayClick(image.ID)}
						>
							<img
								src={"https://www.dataplusscience.com/images/MapExample4.PNG"} // TODO: Change to replay canvas
								alt={image.alt}
								style={{
								width: '190px', // Set the desired width
								height: '190px', // Set the desired height
								objectFit: 'cover', // Adjust how the image fits within the container
								}}
							/>
							<p className="image-caption">{image.name}</p>
							<div className="overlay-grey_ReplayCarousel">
								<img
								src={"https://www.transparentpng.com/thumb/play-button/rqkcPN-play-button-cut-out.png"}
								style={{
									width: '100px', // Set the desired width
									height: '100px', // Set the desired height
									objectFit: 'cover', // Adjust how the image fits within the container
								}}
								/>
							</div>
						</button>
					</Link>
				</div>
			))}
		</Slider>
	);
};

export default ReplayCarousel;