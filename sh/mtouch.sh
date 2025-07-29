#!/bin/bash


# 获取目标路径
read -p "目标路径: " target_path


# 获取新的命名空间
read -p "命名空间: " new_namespace

# 获取新的类名
read -p "类名: " new_class_name


# 字符串处理函数
process_string() {
    echo "$1" |
    tr '[:lower:]' '[:upper:]' |  # 转大写
    sed 's/::/_/g'                # 替换::
}

# 处理各个字符串
processed_ns=$(process_string "$new_namespace")
processed_cls=$(process_string "$new_class_name")

# 拼接最终结果
combined="${processed_ns}_${processed_cls}"

# 处理目标目录（转换 ./ 为当前路径）
if [[ "$target_path" == "./" ]]; then
    target_path=$(pwd)
else
    # 创建目标目录（自动创建父目录）
    mkdir -p "$target_pathtarget_path"
fi

# 遍历 ~/tmp 目录下的所有文件
for file in /root/code_template/c++_template/*; do
    if [ -f "$file" ]; then
        # 获取文件名和扩展名
        filename=$(basename "$file")
        extension="${filename##*.}"
        base="${filename%.*}"

        # 替换前缀
        new_filename="${new_class_name}.$extension"
        
        # 拷贝文件到目标路径
        cp "$file" "$target_path/$new_filename"


        # 使用 sed 进行替换
        sed -i "s/codenamespace/${new_namespace}/g" "$target_path/$new_filename"
        sed -i "s/CodeTemplate/${new_class_name}/g" "$target_path/$new_filename"
        sed -i "s/CODETEMPLATEHH/${combined}/g" "$target_path/$new_filename"
	sed -i "s/current_time/$(date +%F)/g" "$target_path/$new_filename"
        
        echo "Processed $filename -> $new_filename"
    fi
done

echo "All files have been processed."
