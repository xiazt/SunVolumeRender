//
// Created by 孙万捷 on 16/5/22.
//

#ifndef SUNVOLUMERENDER_WOODCOCK_TRACKING_H
#define SUNVOLUMERENDER_WOODCOCK_TRACKING_H

#define GLM_FORCE_INLINE
#include <glm/glm.hpp>

#include <curand.h>
#include <curand_kernel.h>

#include "cuda_ray.h"
#include "cuda_volume.h"
#include "cuda_transfer_function.h"

#define BASE_SAMPLE_STEP_SIZE 150.f

__inline__ __device__ float opacity_to_sigmat(float opacity)
{
    return -logf(1.f - opacity) * BASE_SAMPLE_STEP_SIZE;
}

__inline__ __device__ float sample_distance(const cudaRay& ray, const cudaVolume& volume, const cudaTransferFunction& tf, float invSigmaMax, curandState& rng)
{
    float t = ray.tMin;
    while(true)
    {
        t += -logf(curand_uniform(&rng)) * invSigmaMax;

        auto ptInWorld = ray.PointOnRay(t);
        if(!volume.IsInside(ptInWorld))
            return -FLT_MAX;

        auto intensity = volume(ptInWorld);
        auto color_opacity = tf(intensity);
        auto sigma_t = opacity_to_sigmat(color_opacity.w);

        if(curand_uniform(&rng) < sigma_t * invSigmaMax)
            break;
    }

    return t;
}

#endif //SUNVOLUMERENDER_WOODCOCK_TRACKING_H