import React from 'react';
import Slider from 'react-slick';
import 'slick-carousel/slick/slick.css';
import 'slick-carousel/slick/slick-theme.css';
import './RoverCarousel.css';

const RoverCarousel = ({ rovers }) => {
  const settings = {
    dots: true, // Show navigation dots
    infinite: false, // Loop through the images
    speed: 500, // Transition speed
    slidesToShow: 6, // Number of slides to show at once
    slidesToScroll: 1, // Number of slides to scroll at once
    focusOnSelect: true
  };

  //handleClick is our event handler for the button click
  const handleRoverClick = (RoverMAC) => {
    console.log("SELECTED " + RoverMAC)
  };

  /*
  // Select Rover
  return (
    <Slider {...settings}>
      <div>
        <button 
          onClick={() => handleRoverClick('rover1')}
          className="carousel-button">
          <img 
            src="https://www.lifepng.com/wp-content/uploads/2020/11/Robot-Sideview-png-hd.png" 
            alt="Image 1" 
            style={{
              width: '200px', // Set the desired width
              height: '200px', // Set the desired height
              objectFit: 'cover', // Adjust how the image fits within the container
            }}
          />
          <div className="overlay">
            <p>CONNECT</p>
          </div>
        </button>
      </div>
      <div>
        <button 
          onClick={() => handleRoverClick('rover2')}
          className="carousel-button">
          <img 
            src="https://www.lifepng.com/wp-content/uploads/2020/11/Robot-Sideview-png-hd.png" 
            alt="Image 2" 
            style={{
              width: '200px', // Set the desired width
              height: '200px', // Set the desired height
              objectFit: 'cover', // Adjust how the image fits within the container
            }}
          />
          <div className="overlay">
            <p>OFFLINE</p>
          </div>
        </button>
      </div>
      <div>
        <button 
          onClick={() => handleRoverClick('rover3')}
          className="carousel-button">
          <img 
            src="https://www.lifepng.com/wp-content/uploads/2020/11/Robot-Sideview-png-hd.png" 
            alt="Image 3" 
            style={{
              width: '200px', // Set the desired width
              height: '200px', // Set the desired height
              objectFit: 'cover', // Adjust how the image fits within the container
            }}
          />
          <div className="overlay">
            <p>OFFLINE</p>
          </div>
        </button>
      </div>
    </Slider>
  );
};
*/

return (
  <div style={{ position: 'fixed', left: 0, top: 0, width: '100%', height: '100%' }}>
    <Slider {...settings}>
      {rovers.map((image, index) => (
        <div key={index}>
          <button
            className="carousel-button"
            onClick={() => handleRoverClick(image.MAC)}
          >
            <img 
              src={"https://www.lifepng.com/wp-content/uploads/2020/11/Robot-Sideview-png-hd.png"} 
              alt={image.alt} 
            />
            <p className="image-caption">{image.nickname}</p>
            <div className="overlay">
              <p>{image.overlayText}</p>
            </div>
          </button>
        </div>
      ))}
    </Slider>
  </div>
);
};

export default RoverCarousel;
