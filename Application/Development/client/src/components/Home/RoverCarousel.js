import React from 'react';
import Slider from 'react-slick';
import 'slick-carousel/slick/slick.css';
import 'slick-carousel/slick/slick-theme.css';
import './RoverCarousel.css';
import { BrowserRouter as Router, Routes, Route, Navigate, Link } from 'react-router-dom';

const RoverCarousel = ({ rovers }) => {
  const settings = {
    dots: true,
    infinite: false,
    speed: 500,
    slidesToShow: 6,
    slidesToScroll: 1,
    focusOnSelect: true
  };

  const handleRoverClick = (RoverMAC, RoverNickname) => {
    console.log("SELECTED " + RoverMAC)
    localStorage.setItem('MAC', RoverMAC);
    localStorage.setItem('nickname', RoverNickname);
    //switch to Connected page
  };

  const getOverlayStyle = (overlayText) => {
    if (overlayText === 'CONNECTED') {
      return 'overlay-green_RoverCarousel';
    }
    else if (overlayText === 'OFFLINE') {
      return 'overlay-red_RoverCarousel';
    }
    // Default style for other overlayText values
    return {};
  };

  return (
    <Slider {...settings}>
      {rovers.map((image, index) => (
        <div key={index} className="carousel-item_RoverCarousel">
          <Link to="/connected" className="page-link" draggable={false}>
            <button
              className="carousel-button_RoverCarousel"
              onClick={() => handleRoverClick(image.MAC, image.nickname)}
            >
              <img
                src={"https://www.lifepng.com/wp-content/uploads/2020/11/Robot-Sideview-png-hd.png"}
                alt={image.alt}
                style={{
                  width: '200px', // Set the desired width
                  height: '200px', // Set the desired height
                  objectFit: 'cover', // Adjust how the image fits within the container
                }}
              />
              <p className="image-caption_RoverCarousel">{image.nickname}</p>
              <div className={getOverlayStyle(image.overlayText)}>
                <p>{image.overlayText}</p>
              </div>
            </button>
          </Link>
        </div>
      ))}
    </Slider>
  );
};

export default RoverCarousel;
