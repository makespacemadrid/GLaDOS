import './Header.css';
import logo from './assets/logo.webp';
import fbIcon from './assets/icons8-facebook-60.svg';
import xIcon from './assets/icons8-twitterx-50.svg';
import ytIcon from './assets/icons8-instagram-50.svg';
import flickrIcon from './assets/icons8-flickr-50.svg';
import instaIcon from './assets/icons8-instagram-50.svg';
import githubIcon from './assets/icons8-github-50.svg';

function MakerSpaceHeader() {
    return (
        <header className="MakerSpaceHeader">
            <div class="row">
                <img
                    alt="logo"
                    id="logo"
                    src={logo}/>
            </div> 
            <div class="row" id="socials-list">
                <div class="social-item" id="facebook-button">
                    <a href="http://www.facebook.com/makespacemadrid">
                        <img 
                            alt="Facebook link"
                            class="social-img"
                            src={fbIcon}/>
                    </a>
                </div>
                <div class="social-item" id="x-button">
                    <a href="http://www.x.com/makespacemadrid">
                        <img 
                            alt="X's link"
                            class="social-img"
                            src={xIcon}/>
                    </a>
                </div>
                <div class="social-item" id="youtube-button">
                    <a href="http://www.youtube.com/makespacemadridorg">
                        <img
                            alt="Youtube's link"
                            class="social-img"
                            src={ytIcon}/>
                    </a>
                </div>
                <div class="social-item" id="flickr-button">
                    <a href="https://www.flickr.com/photos/makespacemadrid/">
                        <img
                            alt="Flickr's link"
                            class="social-img"
                            src={flickrIcon}/>
                    </a>
                </div>
                <div class="social-item" id="insta-button">
                    <a href="https://www.instagram.com/makespacemadrid">
                        <img
                            alt="Instagram's link"
                            class="social-img" 
                            src={instaIcon}/>
                    </a>
                </div>
                <div class="social-item" id="github-button">
                    <a href="https://github.com/makespacemadrid">
                        <img 
                            alt="Github's link"
                            class="social-img" 
                            src={githubIcon}/>
                    </a>
                </div>
            </div>
        </header>
    );
}

export default MakerSpaceHeader;
