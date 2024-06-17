#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "hittable.h"
#include "material.h"

class camera {
    private:
        int image_height;
        double pixel_samples_scale;
        point3 center;
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;

        void initialize() {
            image_height = std::max(int(image_width / aspect_ratio),1);  // image height must be at least 1
            center = point3(0,0,0);
            pixel_samples_scale = 1.0 / samples_per_pixel;

            // Viewport Dimensions
            auto focal_length = 1.0;
            auto viewport_height = 2.0;
            auto viewport_width = viewport_height * (double(image_width) / image_height);

            // Viewport Edge Vectors
            auto viewport_u = vec3(viewport_width, 0, 0);   // vector starts top left and goes L->R
            auto viewport_v = vec3(0, -viewport_height, 0); // vector starts top left and goes top to bottom

            // Distance Between Pixels
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            // Starting Pixel (top left)
            auto viewport_upper_left = center
                                    - vec3(0, 0, focal_length)
                                    - viewport_u / 2
                                    - viewport_v / 2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
                    
        }

        color ray_color(const ray& r, int depth, const hittable& world) const {
            if (depth <= 0) { // prevent recursion from exceeding stack
                return color(0,0,0);
            }
            hit_record rec;
            if (world.hit(r, interval(0.001, infinity), rec)) {  // we use 0.001 to avoid errors introduced by floating point rounding
                ray scattered;
                color attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered)) {
                    return attenuation * ray_color(scattered, depth-1, world);
                }
                return color(0,0,0);
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5*(unit_direction.y() + 1.0);
            // linear interpolation takes form (1-a) * startvalue + a*endvalue
            // this is a linear scale for color where in this case 0 = white, 1 = blue, 0.5 = blend
            return (1.0 - a) * color(1,1,1) + a*color(0.5,0.7,1);
        }
        
    public:
        double aspect_ratio = 1;
        int image_width = 400;
        int samples_per_pixel = 10; // number of random samples for each pixel
        int max_depth = 10;

        void render(const hittable& world) {
            initialize();

            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

            for (int j = 0; j < image_height; j++) {
                std::clog << "\rScanlines remaining: " << (image_height-j) << ' ' << std::flush;
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0,0,0);
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    write_color(std::cout, pixel_samples_scale * pixel_color);
                }
            }
            std::clog << "\rDone.                   \n";
        }

        ray get_ray(int i, int j) const {
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc
                                + ((i + offset.x()) * pixel_delta_u)
                                + ((j + offset.y()) * pixel_delta_v);
            auto ray_origin = center;
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
                                 
        }

        vec3 sample_square() const {
            // returns a vector to a random point in [-.5, -.5]-[+.5,+.5]
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }
};

#endif