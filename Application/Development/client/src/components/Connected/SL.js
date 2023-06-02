import React from 'react';
import './grid_Connected.css';

const SL_Button = () => {
  const handleClick = () => {
    console.log('SL Button clicked');
  };

  return (
    <button onClick={handleClick} className='box-green SmallLeftButton_Connected'>
      SL
    </button>
  );
};

export default SL_Button;
