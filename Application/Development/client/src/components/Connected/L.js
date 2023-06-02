import React from 'react';
import './grid_Connected.css';

const L_Button = () => {
  const handleClick = () => {
    console.log('L Button clicked');
  };

  return (
    <button onClick={handleClick} className='box-green LargeButton_Connected'>
      L
    </button>
  );
};

export default L_Button;
