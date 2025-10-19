#!/bin/bash

# ZMQ Simple Docker 启动脚本

set -e

echo "=========================================="
echo "  ZMQ Simple Docker 部署脚本"
echo "=========================================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# 检查是否在 docker 目录
if [ ! -f "docker-compose.yml" ]; then
    echo -e "${RED}错误: 请在 docker 目录中运行此脚本${NC}"
    echo "使用方法: cd docker && ./start.sh"
    exit 1
fi

# 显示菜单
show_menu() {
    echo "请选择操作:"
    echo "1) 构建并启动所有服务（前台运行）"
    echo "2) 构建并启动所有服务（后台运行）"
    echo "3) 停止所有服务"
    echo "4) 查看日志"
    echo "5) 重启服务"
    echo "6) 清理（包括 volume）"
    echo "7) 查看状态"
    echo "8) 进入容器 shell"
    echo "0) 退出"
    echo ""
}

# 构建并启动（前台）
start_foreground() {
    echo -e "${GREEN}正在构建并启动服务...${NC}"
    docker-compose up --build
}

# 构建并启动（后台）
start_background() {
    echo -e "${GREEN}正在构建并启动服务（后台）...${NC}"
    docker-compose up -d --build
    echo ""
    echo -e "${GREEN}服务已启动！${NC}"
    echo "使用以下命令查看日志："
    echo "  docker-compose logs -f"
    echo ""
    docker-compose ps
}

# 停止服务
stop_services() {
    echo -e "${YELLOW}正在停止服务...${NC}"
    docker-compose down
    echo -e "${GREEN}服务已停止${NC}"
}

# 查看日志
view_logs() {
    echo "选择要查看的服务:"
    echo "1) 所有服务"
    echo "2) App A (json_publisher)"
    echo "3) App B (json_subscriber)"
    echo "4) App C (json_app_c.py)"
    read -p "请选择 [1-4]: " log_choice
    
    case $log_choice in
        1) docker-compose logs -f ;;
        2) docker-compose logs -f app_a ;;
        3) docker-compose logs -f app_b ;;
        4) docker-compose logs -f app_c ;;
        *) echo -e "${RED}无效选择${NC}" ;;
    esac
}

# 重启服务
restart_services() {
    echo "选择要重启的服务:"
    echo "1) 所有服务"
    echo "2) App A"
    echo "3) App B"
    echo "4) App C"
    read -p "请选择 [1-4]: " restart_choice
    
    case $restart_choice in
        1) docker-compose restart ;;
        2) docker-compose restart app_a ;;
        3) docker-compose restart app_b ;;
        4) docker-compose restart app_c ;;
        *) echo -e "${RED}无效选择${NC}" ;;
    esac
}

# 清理
cleanup() {
    echo -e "${RED}警告: 这将删除所有容器、网络和 volume！${NC}"
    read -p "确定要继续吗？ (y/N): " confirm
    
    if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]; then
        docker-compose down -v
        echo -e "${GREEN}清理完成${NC}"
    else
        echo "已取消"
    fi
}

# 查看状态
view_status() {
    echo -e "${GREEN}容器状态:${NC}"
    docker-compose ps
    echo ""
    echo -e "${GREEN}Volume 信息:${NC}"
    docker volume ls | grep zmq
    echo ""
    echo -e "${GREEN}网络信息:${NC}"
    docker network ls | grep zmq
}

# 进入容器
enter_container() {
    echo "选择要进入的容器:"
    echo "1) App A"
    echo "2) App B"
    echo "3) App C"
    read -p "请选择 [1-3]: " container_choice
    
    case $container_choice in
        1) docker exec -it zmq_app_a bash ;;
        2) docker exec -it zmq_app_b bash ;;
        3) docker exec -it zmq_app_c bash ;;
        *) echo -e "${RED}无效选择${NC}" ;;
    esac
}

# 主循环
while true; do
    show_menu
    read -p "请选择 [0-8]: " choice
    echo ""
    
    case $choice in
        1) start_foreground ;;
        2) start_background ;;
        3) stop_services ;;
        4) view_logs ;;
        5) restart_services ;;
        6) cleanup ;;
        7) view_status ;;
        8) enter_container ;;
        0) 
            echo "再见！"
            exit 0
            ;;
        *)
            echo -e "${RED}无效选择，请重试${NC}"
            echo ""
            ;;
    esac
    
    echo ""
    read -p "按回车键继续..."
    clear
done

