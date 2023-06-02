import React from 'react';
import './grid_Connected.css';

const SR_Button = () => {
  const handleClick = () => {
    console.log('SR Button clicked');
  };

  return (
    <button onClick={handleClick} className='box-red SmallRightButton_Connected'>
      SR
    </button>
  );
};

export default SR_Button;
