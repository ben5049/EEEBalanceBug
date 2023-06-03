import React from 'react';
import './grid_Connected.css';
import { BrowserRouter as Router, Routes, Route, Navigate, Link } from 'react-router-dom';

const SR_Button = () => {
  const handleClick = () => {
    console.log('SR Button clicked');
  };

  return (
    <div className='box-red SmallRightButton_Connected'>
    <Link to='/' className="page-link" draggable={false}>
        <button onClick={handleClick} className='box-red buttons_Connected'>
            Menu
        </button>
    </Link>
    </div>
  );
};

export default SR_Button;
