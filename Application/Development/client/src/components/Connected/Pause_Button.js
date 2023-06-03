import React from 'react';
import './grid_Connected.css';

const Pause_Button_Connected = () => {
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

export default Pause_Button_Connected;
