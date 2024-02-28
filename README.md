<!-- Improved compatibility of back to top link: See: https://github.com/krackalackel02/HPC-Coursework/pull/73 -->
<a name="readme-top"></a>
<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Don't forget to give the project a star!
*** Thanks again! Now go create something AMAZING! :D
-->



<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
<!-- [![MIT License][license-shield]][license-url] -->
<!-- [![LinkedIn][linkedin-shield]][linkedin-url] -->



<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/krackalackel02/HPC-Coursework">
    <img src="./images/logo.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">HPC-Coursework</h3>

  <p align="center">
    High Performance Computing with OpenMP and CBLAS
    <br />
    <a href="https://github.com/krackalackel02/HPC-Coursework"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/krackalackel02/HPC-Coursework">View Demo</a>
    ·
    <a href="https://github.com/krackalackel02/HPC-Coursework/issues">Report Bug</a>
    ·
    <a href="https://github.com/krackalackel02/HPC-Coursework/issues">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <!-- <li><a href="#license">License</a></li> -->
    <!-- <li><a href="#contact">Contact</a></li> -->
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

[![Product Name Screen Shot][product-screenshot]](https://github.com/krackalackel02/HPC-Coursework)

This repository details the HPC coursework that involves the optimisation of a fluid solver in conjunction with the OpenMP and CBlas library.

This project involves the:
* Implementation of a proper project structure
* Creation of a Makefile that has build and clean up methods
* Optimisation of the default provided code with parallelisation with OpenMP

The executable will output the results of the solver into various text files in the "output" folder.


<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With

<!-- This section should list any major frameworks/libraries used to bootstrap your project. Leave any add-ons/plugins for the acknowledgements section. Here are a few examples. -->

<!-- * [![Next][Next.js]][Next-url] -->
* [![makefile][makefile badge]][makefile url]
* [![cpp][cpp badge]][cpp url]
* [![openmp][openmp badge]][openmp url]
* [![doxygen][doxygen badge]][doxygen url]
* [![cblas][cblas badge]][cblas url]
<!-- * [![Cpp][cpp badge]][cpp url] -->
<!-- * [![React][React.js]][React-url]
* [![Vue][Vue.js]][Vue-url]
* [![Angular][Angular.io]][Angular-url]
* [![Svelte][Svelte.dev]][Svelte-url]
* [![Laravel][Laravel.com]][Laravel-url]
* [![Bootstrap][Bootstrap.com]][Bootstrap-url]
* [![JQuery][JQuery.com]][JQuery-url] -->

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

The following will show how to setup the repository locally and run the program

### Prerequisites

This is an example of how to list things you need to use the software and how to install them.
* Boost library
* Cblas library
* Doxygen
* Make / GNU Make

### Installation

_Below is an example of how to install the repository_

<!-- 1. Get a free API Key at [https://example.com](https://example.com) -->
1. Clone the repo
   ```sh
   git clone https://github.com/krackalackel02/HPC-Coursework.git
   ```
<!-- 3. Install NPM packages
   ```sh
   npm install
   ```
4. Enter your API in `config.js`
   ```js
   const API_KEY = 'ENTER YOUR API';
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p> -->



<!-- USAGE EXAMPLES -->
## Usage

<!-- Use this space to show useful examples of how a project can be used. Additional screenshots, code examples and demos work well in this space. You may also link to more resources. -->

_For more instructions, please refer to the [Documentation](https://example.com)_

1. Call make clean to empty any old build files
   ```sh
   make clean
   ```
2. Call make to build main and unit test executable
   ```sh
   make 
   ```
3. Run main and unit test executable
   ```sh
   ./bin/LidDrivenCavitySolver 
   ```
4. View output files and resulting ".txt" documents
   ```sh
   xdg-open ./output/final.txt
   xdg-open ./output/ic.txt
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [x] Iniitalise Git Repo
- [x] Setup project structure with makefile
- [x] Initialise .gitignore file
- [x] Initialise Markdown
- [ ] Start looking into Optimisation
<!-- - [ ] Multi-language Support
    - [ ] Chinese
    - [ ] Spanish -->

See the [open issues](https://github.com/krackalackel02/HPC-Coursework/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- LICENSE
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p> -->



<!-- CONTACT
## Contact

Your Name - [@your_twitter](https://twitter.com/your_username) - email@example.com

Project Link: [https://github.com/your_username/repo_name](https://github.com/your_username/repo_name)

<p align="right">(<a href="#readme-top">back to top</a>)</p> -->



<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

Use this space to list resources you find helpful and would like to give credit to. I've included a few of my favorites to kick things off!

* [Img Shields](https://shields.io)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/krackalackel02/HPC-Coursework.svg?style=for-the-badge
[contributors-url]: https://github.com/krackalackel02/HPC-Coursework/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/krackalackel02/HPC-Coursework.svg?style=for-the-badge
[forks-url]: https://github.com/krackalackel02/HPC-Coursework/network/members
[stars-shield]: https://img.shields.io/github/stars/krackalackel02/HPC-Coursework.svg?style=for-the-badge
[stars-url]: https://github.com/krackalackel02/HPC-Coursework/stargazers
[issues-shield]: https://img.shields.io/github/issues/krackalackel02/HPC-Coursework.svg?style=for-the-badge
[issues-url]: https://github.com/krackalackel02/HPC-Coursework/issues
[product-screenshot]: ./images/screenshot.png


[cpp badge]: https://img.shields.io/badge/C%2B%2B-white?style=for-the-badge&logo=cplusplus&color=%23084a86&link=https%3A%2F%2Fen.cppreference.com%2Fw%2F
[cpp url]: http://cppreference.com/
[makefile url]: https://www.gnu.org/software/make/
[makefile badge]: https://img.shields.io/badge/Makefile-white?style=for-the-badge&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEsAAABJCAMAAABCWCvTAAACB1BMVEVHcEwFBQUNDQ0FBQUDAwMCAgIGBgYICAgMDAyenp4HBwcHBwcDAwOoqKgKCgoDAwMICAgDAwMHBweSkpIGBgY8PDwGBgYICAgpKSkICAgLCwsHBwcDAwMFBQWEhIQICAgHBwcFBQUJCQkPDw+amppSUlKLi4tPT08hISEDAwOYmJh5eXkZGRkpKSkmJiZKSko8PDwxMTEWFhZcXFxnZ2dFRUUjIyMiIiJLS0s6OjoEBAQDAwMICAgNDQ0EBAQ6OjpFRUVpaWkDAwMpKSlaWlpRUVECAgIqKipzc3N5eXkuLi5ubm5XV1c9PT0dHR0ZGRk4ODiSkpItLS0cHBw4ODhnZ2d/f39BQUFcXFxfX19ra2sYGBhRUVE0NDSIiIg+Pj4zMzMDAAL///8DAwMAAAACAgL9/f35+fn6+vrz8/MHBwesrKwFBQXn5+fs7Ozh4eENDQ3V1dXp6enk5OT19fW5ubnJycmKiorS0tL+/v5iYmLc3Nzv7++lpaXx8fHPz8/8/Pza2to4ODi1tbUvLy8ZGRmioqL39/eysrLe3t4REREoKCjX19dra2tnZ2cjIyNLS0vDw8O8vLxCQkIeHh55eXlzc3N9fX2vr6+VlZXLy8teXl40NDSNjY1vb2/BwcGAgICHh4eDg4NYWFi/v79bW1tUVFQUFBTGxsZQUFB2dnZGRkaFhYW0cH3+AAAApnRSTlMASAgYEBNiAgX+Cy0y/kMhPyZO/jj+kh2jhHeeyNLzaXzeVV39xe/Yguj98Y7ij6980Zzi6L2tbue9sPVxjKiLzNy4xtahv2L52rfy7tHjzuvv9ran8eSlxPfD8PeZ1Ozz///////////////////////////////////////////////////////////////////////////////////////////+5AAZEwAACllJREFUWMOMVwlXGksWbkRBVFBBonlJ1DFuR42auGQ5k0kySV72l8nL8pKZ92bmFA02YDfNjsgO0qBssggCosEFXH7kVLO2Rp9T53D6Vlf1V7e++91bBYLUWnMnp+fdvW/7+/v3bt8RNiJ/1hrbbo5OTo5yOtjnjbawep540zihc+j8ame0976AezHS3a8f9GmT3eD78OQPVsuZ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&color=%23767980&link=https%3A%2F%2Fwww.gnu.org%2Fsoftware%2Fmake%2F

[doxygen url]: https://www.doxygen.nl/
[doxygen badge]: https://img.shields.io/badge/Doxygen-white?style=for-the-badge&logo=data:image/jpeg;base64,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&color=8face7

[cblas url]: https://www.netlib.org/blas/
[cblas badge]: https://img.shields.io/badge/BLAS-white?style=for-the-badge&logo=data:image/png;base64,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&color=090d16

[openmp url]: https://www.openmp.org/
[openmp badge]: https://img.shields.io/badge/Open_MP-white?style=for-the-badge&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAJsAAAAyCAMAAAB8rFmYAAAARVBMVEVHcEwIdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH0IdH2BGXldAAAAFnRSTlMAE73tBd+j+1JysgnTgh1hKo83SHich9xJLwAABHNJREFUWMPtmNuWpCAMReUmggiI4v9/6hBAxGv11PTq6YfKQy87WrgJJwnYNB/72Mc+9hvNtMW8zT40bs6Z1o+03vtxEJOmD0MiOxnTOfTPbOOymci+iW8+3DftcjTOyjxOZgdFOF84UX7qv49N5mjMywu2YKy7jtnAqilI921sSkcXlV9gW9jVey2GW0RhjBkEn5nvYuNTdGn2FbZFnlfMKuAZHOppbw0OdMR8E9syn+S2Y+OEVLfIKXAIAi6LEvuZ3IT3HbYkuGG5ZuOzc5Np+X4mlYlwq0UHh+z/lS0FhOkiN07ObFH+tMzGH4bSqkg2G23XX+XAarQvPhQhdCxHPUJ9zcYVK4JLciP4hq2x7JDWpVSGynGQlyPxMSQG0dOuVUy1ZgusnaVSSoqkAmqGWTd6xkrhVH5yFGTKzaHITfk7NqSu2SBITDcnH7EwXeZ8EgPH7lhumIGRKF646/Lg3OuNbV5Jstz8cMc25dVe2sOSsnMoGxOeM3AL1oFgqfiaHqhN1QY8fKBRS2Qkoa6z+AJPC1t6I7NNnxbT3LBRi1e9Def1G45yBucIbGG9W9tTZMIMMEqy5W2oNmiSSQuBLfQTZay2gsUysLJplV+f5ETcBVs7D6Nkh2pYrDvLLeWHTBJuk8I7EpsjxGJMHohgyKGYg1iXh+bC1ieCEV4BckPDq9qL0Wn5jrjhvRiGADaW6x718ae+GiCEI8wK2EgeAK59YWtERslya16xnWJ0ydYXtrHZdhLEgWeuM6aNPLjfSpvc2BxJa5nl9ootyvfE1t2ybdiAZRyvJyJgoYCtpNcAebWy0VQaeErskPfPbMqcdnCXeoNRJYU8tTXuDBORfjUM1QfYSnDnHVtGiNsHkMI9G2dY6HOHceSijdk1T7fK1wODOE6WWGAbqrjhis1UT47NFdtojDDGTPayRUJKttfBDGyqZE7UuYg7h8rYI5sjO53f1t47u+oLIOfcF3S9pkNgY5OtrX9iK70oDvf3bLGfisOSpl4BenO1BI1ZKgVu8bxja3y9MXqDDV6qdi+Mxb+LqbmlSVgf4twucWznNH1kM7Xc3mCDwC0SHR39odWOUDBS/m5qIBg9sllSQ7zBFntLte8VJO+OgY3kX7tUdeeqencxmR/Z0NrFY3t5g63ROJ8XKO11J8t5IfZTNkGcoLbDBhSymhhIeDqljvbIVvbnsXNcs1H7dCamWqZzlpS7cxawhU7VGuFLBIGI49mIlqQpPLMZXm1+rtmsMNPjmX6+Op9CLnh2ONlOVV2ALkPxE1sWXFq7i9ob/C6U3ufDiR0wHD525/rYT50M/vqLgB0VeILLpQ2KUqUEGaU8nVW0kUYtx+u0hxLpRth+DipbaM5o6uwr1Z2/h6Re38PEdl9SkOvAtf5XDjHxQIPiH7D1brnebqxX8URE3znS7fYhv8z+O9s039iU2az4Oev2CeuXG/OZzSw/Z/LIxi9tY+M/Zkc2192Yy2y6+zn7+hcirQj5rXkaOt03fP792Mc+9rb9AeUGuqWYp4OhAAAAAElFTkSuQmCC&color=001f21