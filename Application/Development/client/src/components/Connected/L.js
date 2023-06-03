import React from 'react';
import './grid_Connected.css';

const L_Button = () => {
  const handleClick = () => {
    console.log('Pause');
    // TODO: Send Pause Post to server
  };

  return (
    <button onClick={handleClick} className='box-green LargeButton_Connected'>
        Pause
    </button>
  );
};

export default L_Button;
