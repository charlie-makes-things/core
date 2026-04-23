# Requires shadercross CLI installed from SDL_shadercross

sc='../../tools/SDL_shadercross/build/shadercross'

#echo $sc
cd examples/build/assets/shaders

for filename in *.vert.hlsl; do
    if [ -f "$filename" ]; then
        ./$sc "$filename" -o "compiled/${filename/.hlsl/.spv}"
        ./$sc "$filename" -o "compiled/${filename/.hlsl/.msl}"
        ./$sc "$filename" -o "compiled//${filename/.hlsl/.dxil}"
    fi
done

for filename in *.frag.hlsl; do
    if [ -f "$filename" ]; then
        ./$sc "$filename" -o "compiled//${filename/.hlsl/.spv}"
        ./$sc "$filename" -o "compiled/${filename/.hlsl/.msl}"
        ./$sc "$filename" -o "compiled//${filename/.hlsl/.dxil}"
    fi
done

for filename in *.comp.hlsl; do
    if [ -f "$filename" ]; then
        ./$sc "$filename" -o "compiled//${filename/.hlsl/.spv}"
        ./$sc "$filename" -o "compiled/${filename/.hlsl/.msl}"
        ./$sc "$filename" -o "compiled//${filename/.hlsl/.dxil}"
    fi
done

cd ../../
