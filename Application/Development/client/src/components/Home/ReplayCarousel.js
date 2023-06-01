import React from 'react';
import Slider from 'react-slick';
import 'slick-carousel/slick/slick.css';
import 'slick-carousel/slick/slick-theme.css';

const ReplayCarousel = () => {
  const settings = {
    dots: true, // Show navigation dots
    infinite: false, // Loop through the images
    speed: 500, // Transition speed
    slidesToShow: 3, // Number of slides to show at once
    slidesToScroll: 1, // Number of slides to scroll at once
  };

  //handleClick is our event handler for the button click
  const handleImageClick = (imageNumber) => {
    console.log("SELECTED " + imageNumber)
  };

  // Get number of replays (with timestamp + mac)
  const getReplayList = 2;


  const fetchData = () => {
    return fetch("http://localhost:3001/pollServer/");
  }

  return (
    <Slider {...settings}>
        
            <div>
                <button 
                onClick={() => handleImageClick('rover1')}
                className="carousel-button">
                <img 
                    src="data:image/png;base64,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" 
                    alt="Image 1" 
                    style={{
                    width: '200px', // Set the desired width
                    height: '200px', // Set the desired height
                    objectFit: 'cover', // Adjust how the image fits within the container
                    }}
                />
                </button>
            </div>
    </Slider>
  );
};

export default ReplayCarousel;
