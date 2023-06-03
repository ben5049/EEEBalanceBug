/*
import React from 'react';
import Slider from 'react-slick';
import 'slick-carousel/slick/slick.css';
import 'slick-carousel/slick/slick-theme.css';
import './AddButton.css';
import { BrowserRouter as Router, Routes, Route, Navigate, Link } from 'react-router-dom';

const AddButton = ({ rovers }) => {
    const handleAddRoverClick = () => {
		console.log("Add new rover")
	};

    return (
        <button
            className="carousel-button_RoverCarousel"
            onClick={() => handleAddRoverClick()}
        >
            <img
                src={"https://www.pngmart.com/files/21/Add-Button-PNG-Isolated-File.png"}
                alt={"Add Image"}
                style={{
                    width: '100px', // Set the desired width
                    height: '100px', // Set the desired height
                    objectFit: 'cover', // Adjust how the image fits within the container
                }}
            />
            <div className={overlay}>
                <p>{image.overlayText}</p>
            </div>
        </button>
    );
};

export default AddButton;
*/