import React from 'react';
import { Link } from 'react-router-dom';
import './grid_Connected.css';

const SL_Button = () => {
  const state = localStorage.getItem('ConnectedState');

  const getText = () => {
    if (state === 'Start') {
      return 'Start Mapping';
    } else if (state === 'Pause') {
      return 'Pause';
    } else if (state === 'Finish') {
      return 'View Replay';
    }
  };

  const getLink = () => {
    if (state === 'Finish') {
      return '/replay'; // Set the desired link for "Finish" state
    } else {
      return '#'; // Set the default link for other states
    }
  };

  const handleClick = () => {
    console.log('SL Button clicked');
    if (state === 'Start') {
        // SEND POST [Start];
        
        localStorage.setItem('ConnectedState', 'Mapping');
        // Trigger the storage event to notify other tabs/windows of the change
        console.log("State set to Mapping");
    } else if (state === 'Pause') {
        // SEND POST [PAUSE];
        localStorage.setItem('ConnectedState', 'Mapping');
        // Trigger the storage event to notify other tabs/windows of the change
        console.log("State set to Mapping");
    } else if (state === 'Finish') {
        // Store Replay ID?
        // Switch to Replay page
    }
  };

  return (
    <div className='box-green SmallLeftButton_Connected'>
        <Link to={getLink()} className="page-link" draggable={false}>
            <button onClick={handleClick} className='box-green buttons_Connected'>
                {getText()}
            </button>
        </Link>
    </div>
  );
};


export default SL_Button;
